/*
 * VNC over Jingle code
 * Chris Waters 2009.
 *
 * Based on:
 * Jingle call example
 * Copyright 2004--2005, Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Tempe Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iomanip>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#else
#include <direct.h>
//typedef _getcwd getcwd;
#include "talk/base/win32.h"
#endif

#include "vncjingle.h"
#include "logging.h"

#if defined(_MSC_VER) && (_MSC_VER < 1400)
// The following are necessary to properly link when compiling STL without
// /EHsc, otherwise known as C++ exceptions.
void __cdecl std::_Throw(const std::exception &) {}
std::_Prhand std::_Raise_handler = 0;
#endif

static DebugLog debug_log_;

void VncClient::OnStateChange(buzz::XmppEngine::State state) 
{
  switch (state) {
  case buzz::XmppEngine::STATE_START:
    LOG(INFO) << "Connecting...";
    break;
  case buzz::XmppEngine::STATE_OPENING:
    LOG(INFO) << "Logging in. ";
    break;
  case buzz::XmppEngine::STATE_OPEN:
    LOG(INFO) << "Logged in as " << xmpp_client_->jid().Str();
    OnSignon();
		SignalLoginComplete(true);
    break;
  case buzz::XmppEngine::STATE_CLOSED:
    LOG(INFO) << "Logged out/login failed";
		SignalLoginComplete(false);
    break;
  }
}
//-----------------------------------------------------------------------------

void VncClient::OnJingleInfo(const std::string & relay_token,
  const std::vector<std::string> &relay_addresses,
  const std::vector<talk_base::SocketAddress> &stun_addresses) 
{
  port_allocator_->SetStunHosts(stun_addresses);
  port_allocator_->SetRelayHosts(relay_addresses);
  port_allocator_->SetRelayToken(relay_token);
}
//-----------------------------------------------------------------------------							
  
void VncClient::OnStatusUpdate(const buzz::Status &status) 
{
	LOG(INFO) << "Status update from " << status.jid().Str();
	talk_base::CritScope lock(&roster_lock_);

	// Only consider JIDs which match our own. We don't want to
	// connect to our buddies.
	if (status.jid().BareJid() != connection_->user_jid_.BareJid()) {
		return;
	}

	RosterEntry entry;
	entry.jid = status.jid();
	entry.nickname = status.nickname();
	if (entry.nickname.size() == 0) {
		entry.nickname = entry.jid.Str();
	}

	if (status.available() && status.screenshare_server_capability()) {
		LOG(INFO) << "Adding to roster: " << entry.nickname;
		roster_[entry.nickname] = entry;
	} else {
		LOG(INFO) << "Removing from roster: " << entry.nickname;
		Roster::iterator iter = roster_.find(entry.nickname);
		if (iter != roster_.end()) {
			roster_.erase(iter);
		}
	}
}
//-----------------------------------------------------------------------------
  
void VncClient::OnMessage(talk_base::Message *m) 
{
  ASSERT(m->message_id == MSG_STOP);
  talk_base::Thread *thread = talk_base::ThreadManager::CurrentThread();
  thread->Stop();
}
//-----------------------------------------------------------------------------

void VncClient::OnIncomingTunnel(cricket::TunnelSessionClient* client, buzz::Jid jid,
	std::string description, cricket::Session* session) 
{
  LOG(INFO) << "IncomingTunnel from " << jid.Str() << ": " << description;

	// Only consider JIDs which match our own. We don't want to
	// connect to our buddies.
	if (jid.BareJid() != connection_->user_jid_.BareJid()) {
		return;
	}

	VncJingleSocket *socket = new VncJingleSocket(client->AcceptTunnel(session), 
		session_manager_->signaling_thread());
	SignalAcceptConnection(socket);
}
//-----------------------------------------------------------------------------

void VncClient::OnSignon() 
{
  std::string client_unique = xmpp_client_->jid().Str();
  cricket::InitRandom(client_unique.c_str(), client_unique.size());

	worker_thread_ = new talk_base::Thread();

  buzz::PresencePushTask *presence_push_ = new buzz::PresencePushTask(xmpp_client_);
  presence_push_->SignalStatusUpdate.connect(this, &VncClient::OnStatusUpdate);
  presence_push_->Start();
  
  buzz::Status my_status;
  my_status.set_jid(xmpp_client_->jid());
  my_status.set_available(true);
  my_status.set_show(buzz::Status::SHOW_ONLINE);
  my_status.set_priority(0);
  my_status.set_know_capabilities(true);
	if (is_server_) {
		my_status.set_screenshare_server_capability(true);
	} else {
		my_status.set_screenshare_client_capability(true);
	}
  my_status.set_is_paglo_client(true);
	if (host_name_.size() > 0) {
		my_status.set_nickname(host_name_);
	}
  my_status.set_version("1.0.0.1");

  buzz::PresenceOutTask* presence_out_ =
    new buzz::PresenceOutTask(xmpp_client_);
  presence_out_->Send(my_status);
  presence_out_->Start();
  
  port_allocator_.reset(new cricket::HttpPortAllocator(&network_manager_, "prc"));

  session_manager_.reset(new cricket::SessionManager(port_allocator_.get(), worker_thread_));

  cricket::SessionManagerTask * session_manager_task = new cricket::SessionManagerTask(xmpp_client_, session_manager_.get());
  session_manager_task->EnableOutgoingMessages();
  session_manager_task->Start();
  
	/*
	 * Initiate ICE negotiation.
	 */
  buzz::JingleInfoTask *jingle_info_task = new buzz::JingleInfoTask(xmpp_client_);
  jingle_info_task->RefreshJingleInfoNow();
  jingle_info_task->SignalJingleInfo.connect(this, &VncClient::OnJingleInfo);
  jingle_info_task->Start();
  
	tunnel_client_.reset(new cricket::TunnelSessionClient(xmpp_client_->jid(), session_manager_.get()));
  tunnel_client_.get()->SignalIncomingTunnel.connect(this, &VncClient::OnIncomingTunnel);

	worker_thread_->Start();
}
//-----------------------------------------------------------------------------

VncJingleConnection::VncJingleConnection(bool is_server, std::string host_name) :
	is_server_(is_server),
	host_name_(host_name),
	debug_(true),
	pump(NULL),
	vnc_client(NULL)
{
	if (debug_) {
    talk_base::LogMessage::LogToDebug(talk_base::LS_VERBOSE);
	} else {
    talk_base::LogMessage::LogToDebug(talk_base::LS_ERROR + 1);
	}

  talk_base::InitializeSSL();   

	ss = new talk_base::PhysicalSocketServer();

  main_thread = new talk_base::Thread(ss);
  talk_base::ThreadManager::SetCurrent(main_thread);

  main_thread->Start();
}
//-----------------------------------------------------------------------------

VncJingleConnection::~VncJingleConnection()
{
	delete main_thread;
	if (vnc_client) {
		delete vnc_client;
	}
	if (pump) {
		delete pump;
	}
}
//-----------------------------------------------------------------------------

void VncJingleConnection::Login(std::string username, std::string password)
{
	if (pump) {
		/*
		 * Disconnect any existing client.
		 */
		pump->client()->Disconnect();

		/* TODO: Not sure how to cleanly free old state. */
	}

	pump = new XmppPump();

  if (debug_) {
    pump->client()->SignalLogInput.connect(&debug_log_, &DebugLog::Input);
    pump->client()->SignalLogOutput.connect(&debug_log_, &DebugLog::Output);
  }

  vnc_client = new VncClient(this, pump->client(), is_server_, host_name_);

  pump->client()->SignalStateChange.connect(vnc_client, &VncClient::OnStateChange);
	vnc_client->SignalAcceptConnection.connect(this, &VncJingleConnection::OnAcceptConnection);
	vnc_client->SignalLoginComplete.connect(this, &VncJingleConnection::OnLoginComplete);

	talk_base::InsecureCryptStringImpl pass;
	pass.password() = password;

	user_jid_ = buzz::Jid(username);
	xcs.set_user(user_jid_.node());
	xcs.set_resource("prc");
	xcs.set_host(user_jid_.domain());
	xcs.set_use_tls(true);
  xcs.set_pass(talk_base::CryptString(pass));
	xcs.set_server(talk_base::SocketAddress("talk.google.com", 5222));

  pump->DoLogin(xcs, new XmppSocket(true), NULL);
}
//-----------------------------------------------------------------------------

VncJingleSocket *VncJingleConnection::Connect(std::string peer_name)
{
	buzz::Jid peer_jid(peer_name);

	//vnc_client->SetPeerJid(peer_jid);
	talk_base::StreamInterface *stream = vnc_client->GetTunnelClient()->CreateTunnel(peer_jid, "pipetcp:5900");
	VncJingleSocket *sock = new VncJingleSocket(stream, vnc_client->SessionManager()->signaling_thread());

	return sock;
}
//-----------------------------------------------------------------------------

void VncJingleConnection::OnLoginComplete(bool Connected)
{
	SignalLoginComplete(Connected);
}
//-----------------------------------------------------------------------------

void VncJingleConnection::OnAcceptConnection(VncJingleSocket *socket)
{
	SignalAcceptConnection(socket);
}
//-----------------------------------------------------------------------------

enum {
	VNC_JINGLE_SOCKET_WRITE,
	VNC_JINGLE_SOCKET_CLOSE
};

VncJingleSocket::VncJingleSocket(talk_base::StreamInterface *stream,
	talk_base::Thread *thread) 
	: stream_(stream), thread_(thread)
{
	int buffer_size = 2 * 1024 * 1024;
	recv_buffer_ = new vncjingle::CircularBuffer(buffer_size);
	send_buffer_ = new vncjingle::CircularBuffer(buffer_size);
	connected_ = false;

	/*
	 * Register to receive events.
	 */
	stream_->SignalEvent.connect(this, &VncJingleSocket::OnStreamEvent);
}
//-----------------------------------------------------------------------------

VncJingleSocket::~VncJingleSocket()
{
	delete recv_buffer_;
	delete send_buffer_;
}
//-----------------------------------------------------------------------------

int VncJingleSocket::Recv(const char *buf, int len, int flags)
{
	// Can't receive data in worker thread otherwise we would deadlock.
//	ASSERT(talk_base::ThreadManager::CurrentThread() != thread_);

	if (!recv_buffer_->Read(buf, len, (flags & MSG_NBLOCK) == 0, (flags & MSG_PEEK) != 0)) {
		errno = EWOULDBLOCK;
		return -1;
	}

	//LOG(INFO) << "Recv bytes: " << len;

	return len;
}
//-----------------------------------------------------------------------------

int VncJingleSocket::Send(const char *buf, int len, int flags)
{
	// Can't send data in worker thread otherwise we would deadlock.
//	ASSERT(talk_base::ThreadManager::CurrentThread() != thread_);

	errno = 0;

	send_buffer_->Write(buf, len);

	/*
	 * Poke the queue to send the data.
	 */
	thread_->Post(this, VNC_JINGLE_SOCKET_WRITE);

	//LOG(INFO) << "Send bytes: " << len;

	return len;
}
//-----------------------------------------------------------------------------

int VncJingleSocket::GetLastError()
{
	return 0;
}
//-----------------------------------------------------------------------------

int VncJingleSocket::SetSockOpt(int level, int opt_name, const char *opt_val, int opt_len)
{
	// TODO: Disable nagle algorithm in pseudotcp.cc

	return 0;
}
//-----------------------------------------------------------------------------

void VncJingleSocket::OnStreamEvent(talk_base::StreamInterface *stream, int events, int error) 
{
	ASSERT(talk_base::ThreadManager::CurrentThread() == thread_);

	if (events & talk_base::SE_CLOSE) {
		connected_ = false;
    if (error == 0) {
      LOG(INFO) << "Socket closed";
    } else {
			LOG(INFO) << "Socket closed with error: " << error;
    }
		CloseSocket();
		return;
	}

  if (events & talk_base::SE_OPEN) {
		LOG(INFO) << "Socket connected";
		connected_ = true;
  }
  if ((events & talk_base::SE_WRITE)) {
		//LOG(INFO) << "Write event";
		WriteQueuedData();
  }
  if ((events & talk_base::SE_READ)) {
	  //LOG(INFO) << "Read event";

		/*
		 * Read the data into our queue.
		 */
		while (true) {
			size_t count;
			int error;
			talk_base::StreamResult result;
			char buffer[32 * 1024];

			result = stream_->Read(buffer, 32 * 1024, &count, &error);
			switch (result) {
			case talk_base::SR_SUCCESS:
				//LOG(INFO) << "Read bytes: " << count;
				recv_buffer_->Write(buffer, count);
				break;

			case talk_base::SR_ERROR:
				LOG(LERROR) << "Error on recv: " << error;
				CloseSocket();
				return;

			case talk_base::SR_EOS:
				LOG(LERROR) << "End of stream on recv";
				CloseSocket();
				return;
				
			case talk_base::SR_BLOCK:
				//LOG(INFO) << "Blocked";
				// No more data to read.
				return;

			default:
				LOG(LERROR) << "Recv failed";
				CloseSocket();
				return;
			}
		}
	}
}
//-----------------------------------------------------------------------------

void VncJingleSocket::Close()
{
	thread_->Post(this, VNC_JINGLE_SOCKET_CLOSE);
}
//-----------------------------------------------------------------------------

void VncJingleSocket::WriteQueuedData()
{
	if (!stream_) {
		return;
	}

	int available_len;
	while ((available_len = send_buffer_->Length()) > 0) {
		talk_base::StreamResult result;
		int error;
		size_t count_in = 2000, count = 0;
		char buffer[2000];
		if (available_len < (int)count_in) {
			count_in = available_len;
		}
		if (!send_buffer_->Read(buffer, count_in, false, true)) {
			return;
		}

		result = stream_->Write(buffer, count_in, &count, &error);

		switch (result) {
		case talk_base::SR_SUCCESS:
			//LOG(INFO) << "Sent " << count << " bytes to peer";
			send_buffer_->ReadSkip(count);
			break;

		case talk_base::SR_BLOCK:
			LOG(INFO) << "Send blocked";
			send_buffer_->ReadSkip(count);
			return;

		case talk_base::SR_EOS:
			CloseSocket();
			return;
		
		case talk_base::SR_ERROR:
			if (error == WSAENOTCONN) {
				LOG(INFO) << "Send failed - not connected yet";
			} else {
				LOG(WARNING) << "Send failed - " << error;
				CloseSocket();
			}
			return;

		default:
			LOG(LERROR) << "Send failed";
			CloseSocket();
			return;
		} 
	}
}
//-----------------------------------------------------------------------------

void VncJingleSocket::CloseSocket()
{
	if (stream_) {
		stream_->Close();
		stream_ = NULL;
	}
	recv_buffer_->Done();
	send_buffer_->Done();
}
//-----------------------------------------------------------------------------

void VncJingleSocket::OnMessage(talk_base::Message *msg) 
{
	// Only handle messages in the worker thread.
	ASSERT(talk_base::ThreadManager::CurrentThread() == thread_);

	switch (msg->message_id) {
	case VNC_JINGLE_SOCKET_WRITE:
		WriteQueuedData();
		break;

	case VNC_JINGLE_SOCKET_CLOSE:
		CloseSocket();
		break;
	}
}
//-----------------------------------------------------------------------------

VncRosterProxy::VncRosterProxy(Roster *roster,
	talk_base::CriticalSection *roster_lock)
	: roster_(roster), roster_lock_(roster_lock)
{
}
//-----------------------------------------------------------------------------

int VncRosterProxy::Length()
{
	talk_base::CritScope lock(roster_lock_);

	return roster_->size();
}
//-----------------------------------------------------------------------------

RosterEntry VncRosterProxy::Get(int index)
{
	talk_base::CritScope lock(roster_lock_);
	int current = 0;

	for (Roster::iterator iter = roster_->begin(); 
		iter != roster_->end(); iter++, current++) {
		if (current == index) {
			return (*iter).second;
		}
	}

	RosterEntry blank;
	return blank;
}
//-----------------------------------------------------------------------------

std::string VncRosterProxy::GetName(int index)
{
	talk_base::CritScope lock(roster_lock_);
	int current = 0;

	for (Roster::iterator iter = roster_->begin(); 
		iter != roster_->end(); iter++, current++) {
		if (current == index) {
			RosterEntry entry = (*iter).second;

			std::string name = entry.nickname;
			if (name.empty()) {
				name = entry.jid.Str();
			}

			return name;
		}
	}

	return "";
}
//-----------------------------------------------------------------------------

RosterEntry VncRosterProxy::GetByName(std::string Name)
{
	talk_base::CritScope lock(roster_lock_);
	RosterEntry r;
	r = (*roster_)[Name];

	return r;
}
//-----------------------------------------------------------------------------
