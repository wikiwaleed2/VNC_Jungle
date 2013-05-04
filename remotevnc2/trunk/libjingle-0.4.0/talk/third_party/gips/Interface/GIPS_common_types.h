

// GIPS_common_types.h

#if (defined(_WIN32_WCE) && !defined(GIPS_WCE))
#define GIPS_CHAR TCHAR         // Windows CE is UNICODE based
#else
#define GIPS_CHAR char
#endif

#ifndef GIPS_COMMON_TYPES_H
#define GIPS_COMMON_TYPES_H


#define FILE_FORMAT_PCM_FILE 0
#define FILE_FORMAT_WAV_FILE 1
#define FILE_FORMAT_COMPRESSED_FILE 2
#define FILE_FORMAT_AVI_FILE 3

// Enum parameter for StartRecordingCallStereo and StopRecordingCallStereo()
enum stereo_record 
{
        left = 0,right = 1,both = 2
};

// GIPS_encryption
// This is a class that should be overloaded to enable encryption

class GIPS_encryption
{
public:
    virtual void encrypt(int channel_no, unsigned char * in_data, 
                unsigned char * out_data, int bytes_in, int * bytes_out) = 0;

    virtual void decrypt(int channel_no, unsigned char * in_data, 
                unsigned char * out_data, int bytes_in, int * bytes_out) = 0;

    virtual void encrypt_rtcp(int channel_no, unsigned char * in_data, 
                unsigned char * out_data, int bytes_in, int * bytes_out) = 0;

    virtual void decrypt_rtcp(int channel_no, unsigned char * in_data, 
                unsigned char * out_data, int bytes_in, int * bytes_out) = 0;
    virtual ~GIPS_encryption() {}
protected:
   GIPS_encryption() {} 
};

// External transport protocol
// This is a class that should be implemented by the customer IF
// a different transport protocol than IP/UDP/RTP is wanted. The
// standard data transport protocol used by VoiceEngine is IP/UDP/RTP
// according to the SIP-standard.
class GIPS_transport
{
public:
    virtual int SendPacket(int channel, const void *data, int len) = 0;
    virtual int SendRTCPPacket(int channel, const void *data, int len) = 0;
    virtual ~GIPS_transport() {}
protected:
   GIPS_transport() {}
};



//////////////////////////////////////////////////////////////////////
// GIPS_CodecInst
//
// Each codec supported by the GIPS Conference Engine can be 
// described by this structure.
//
// The following codecs are supported today (if included in the delivery ):
//
// - G.711 u-Law
// - G.711 a-Law
// - GIPS Enhanced G.711 u-Law
// - GIPS Enhanced G.711 A-Law
// - GIPS iPCM-wb
// - GIPS iLBC
// - GIPS iSAC
// - G.729 (The user of GIPS Conference Engine is responsible for licensing of IPR for this codec) 
//
//
// Note that the GIPS NetEQ is included on the receiving side for all
// codecs. NetEQ is a patented GIPS technology that adaptively compen-,
// sates for jitter, and at the same time conceals errors due to lost
// packets. NetEQ delivers improvements in sound quality while mini-
// mizing buffering latency.
//////////////////////////////////////////////////////////////////////

struct GIPS_CodecInst
{
        int pltype;
        GIPS_CHAR plname[32];
        int plfreq;
        int pacsize;
        int channels;
        int rate;
};


// External read and write functions
// This is a class that should be implemented by the customer IF
// the functions GIPS***_ConvertWavToPCM() or GIPS***_PlayPCM() are used

class  InStream 
{
public:
        virtual int Read(void *buf,int len) = 0;
        // len - size in bytes that should be read
        // returns the size in bytes read (=len before end and =[0..len-1] at end similar to fread)

        // Called when a wav-file needs to be rewinded
        virtual int Rewind() {return -1;}

        // Destructor
        virtual ~InStream() {};
};

class  OutStream 
{
public:
        virtual bool Write(const void *buf,int len) = 0;
        // true for ok, false for fail

        // Only internal usage
        virtual int Rewind() {return -1;}

        // Destructor
        virtual ~OutStream() {};
};


#endif
