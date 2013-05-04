#ifndef _VNCJINGLE_H_
#define _VNCJINGLE_H_

#include "talk/base/sigslot.h"
#include "talk/base/stream.h"
#include "talk/base/fileutils.h"
#include "talk/base/pathutils.h"
#include "talk/base/helpers.h"
#include "talk/base/httpclient.h"
#include "talk/base/logging.h"
#include "talk/base/bytebuffer.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/ssladapter.h"
#include "talk/xmpp/xmppclientsettings.h"
#include "xmppthread.h"
#include "xmppauth.h"
#include "talk/p2p/client/httpportallocator.h"
#include "talk/p2p/client/sessionmanagertask.h"
#include "talk/session/tunnel/tunnelsessionclient.h"
#include "presencepushtask.h"
#include "presenceouttask.h"
#include "jingleinfotask.h"
#include "circularbuffer.h"
#include <map>

/*
 * Custom non-block flag that can be used on individual network calls.
 */
#define MSG_NBLOCK 0x800000

/*
 * Class implememnting a psuedo TCP socket over the P2P network.
 */
class VncJingleSocket : public sigslot::has_slots<>, public talk_base::MessageHandler {
public:
	VncJingleSocket(talk_base::StreamInterface *stream, talk_base::Thread *thread);
	~VncJingleSocket();

	/*
	 * Send or receive. Blocking by default. 
	 *
	 * Supported flags are:
	 *    MSG_PEEK - Read data without removing it from recv buffer.
	 *    MSG_NBLOCK - Perform a non-blocking request. Returns -1 if request
	 *       would have blocked and sets errno to EWOULDBLOCK.
	 *
	 * Recv, Send and Close are thread-safe.
	 */
	int Recv(const char *buf, int len, int flags);
	int Send(const char *buf, int len, int flags);
	void Close();

	int GetLastError();
	int SetSockOpt(int level, int opt_name, const char *opt_val, int opt_len);

	/*
	 * Test if the connection has been closed.
	 */
	bool ConnectionClosed() { return stream_ == NULL; }

	/*
	 * Callbacks.
	 */
	void VncJingleSocket::OnStreamEvent(talk_base::StreamInterface *stream, int events, int error);

protected:
	void WriteQueuedData();
	void CloseSocket();
	void OnMessage(talk_base::Message *msg);

	// Jingle stream which data is transported over.
	talk_base::StreamInterface *stream_;
	// Whether the stream is connected.
	bool connected_; 

	/*
	 * Buffer between the event driving libjingle internals and the blocking/
	 * non-blocking API that this class provides. These structures are thread
	 * safe.
	 */
	vncjingle::CircularBuffer *recv_buffer_;
	vncjingle::CircularBuffer *send_buffer_;

	/*
	 * Thread this socket operates in.
	 */
	talk_base::Thread *thread_;
};
//-----------------------------------------------------------------------------

/*
 * A peer which is available for connections.
 */
struct RosterEntry {
  buzz::Jid jid;
	std::string nickname;
};
//-----------------------------------------------------------------------------
typedef std::map<std::string,RosterEntry> Roster;


/*
 * A class for accessing the roster.
 */
class VncRosterProxy {
public:
	VncRosterProxy(Roster *roster, talk_base::CriticalSection *roster_lock);

	/*
	 * Get the number of entries in the roster.
	 */
	int Length();

	/*
	 * Get the nth entry from the roster.
	 */
	RosterEntry Get(int index);
	std::string GetName(int index);
	RosterEntry GetByName(std::string Name);

private:
	Roster *roster_;
	talk_base::CriticalSection *roster_lock_;
};
//-----------------------------------------------------------------------------

class VncJingleConnection;

class VncClient : public sigslot::has_slots<>, public talk_base::MessageHandler {
public:
	VncClient(VncJingleConnection *connection, buzz::XmppClient *xmppclient, bool is_server, std::string host_name) :
		connection_(connection),
    xmpp_client_(xmppclient),
    is_server_(is_server),
		host_name_(host_name)
		{}

  void OnStateChange(buzz::XmppEngine::State state);

	cricket::TunnelSessionClient *GetTunnelClient() { return tunnel_client_.get(); }

	// Signal to indicate a new connection
	sigslot::signal1<VncJingleSocket *> SignalAcceptConnection;
	// Signal to indicate login completion. The parameter indicates login success
	// or failure.
	sigslot::signal1<bool> SignalLoginComplete;

	cricket::SessionManager *SessionManager() { return session_manager_.get(); }

	VncRosterProxy *GetRoster() { return new VncRosterProxy(&roster_, &roster_lock_); }

private:
  enum {
    MSG_STOP,
  };

  void OnJingleInfo(const std::string & relay_token,
    const std::vector<std::string> &relay_addresses,
    const std::vector<talk_base::SocketAddress> &stun_addresses);
  void OnStatusUpdate(const buzz::Status &status);
  void OnMessage(talk_base::Message *m);
	void OnIncomingTunnel(cricket::TunnelSessionClient* client, buzz::Jid jid,
		std::string description, cricket::Session* session);
  void OnSignon();

  talk_base::NetworkManager network_manager_;
  talk_base::scoped_ptr<cricket::HttpPortAllocator> port_allocator_;
  talk_base::scoped_ptr<cricket::SessionManager> session_manager_;
  talk_base::scoped_ptr<cricket::TunnelSessionClient> tunnel_client_;
  buzz::XmppClient *xmpp_client_;
	/*
	 * Whether this instance is a screen server or viewer.
	 */
  bool is_server_;
	/*
	 * Thread which handles the work of data transfer.
	 */
	talk_base::Thread *worker_thread_;

	/*
	 * List of peers which are available for connections.
	 */
	Roster roster_;
	mutable talk_base::CriticalSection roster_lock_;

	/*
	 * Name of this host - human readable, not DNS name.
	 */
	std::string host_name_;

	VncJingleConnection *connection_;
};
//-----------------------------------------------------------------------------

/*
 * A VNC connction over a jingle P2P connection.
 */
class VncJingleConnection : public sigslot::has_slots<> {
public:
	VncJingleConnection(bool is_server, std::string host_name);
	virtual ~VncJingleConnection();

	/*
	 * Login to the connection server. Will trigger SignalLoginComplete when
	 * login succeeds or fails.
	 */
	void Login(std::string username, std::string password);

	/*
	 * Create a data tunnel to the specified peer.
	 */
	VncJingleSocket *Connect(std::string peer_name);

	/*
	 * Accept a connection.
	 */
	void OnAcceptConnection(VncJingleSocket *socket);
	void OnLoginComplete(bool Connected);

	// Signal to indicate a new connection.
	sigslot::signal1<VncJingleSocket *> SignalAcceptConnection;

	// Signal to indicate login completion. The parameter indicates login success
	// or failure.
	sigslot::signal1<bool> SignalLoginComplete;

	VncRosterProxy *GetRoster() { return vnc_client->GetRoster(); }

	/*
	 * Jid of the logged in user.
	 */
	buzz::Jid user_jid_;
protected:

  talk_base::PhysicalSocketServer *ss;
  XmppPump *pump;
  buzz::XmppClientSettings xcs;
	talk_base::Thread *main_thread;
	VncClient *vnc_client;
	bool is_server_;
	std::string host_name_;
	bool debug_;
};
//-----------------------------------------------------------------------------



#endif