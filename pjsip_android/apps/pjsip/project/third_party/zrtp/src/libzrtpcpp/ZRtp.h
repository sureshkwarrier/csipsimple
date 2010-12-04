/*
  Copyright (C) 2006-2009 Werner Dittmann

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ZRTP_H_
#define _ZRTP_H_

#include <cstdlib>

#include <libzrtpcpp/ZrtpPacketHello.h>
#include <libzrtpcpp/ZrtpPacketHelloAck.h>
#include <libzrtpcpp/ZrtpPacketCommit.h>
#include <libzrtpcpp/ZrtpPacketDHPart.h>
#include <libzrtpcpp/ZrtpPacketConfirm.h>
#include <libzrtpcpp/ZrtpPacketConf2Ack.h>
#include <libzrtpcpp/ZrtpPacketGoClear.h>
#include <libzrtpcpp/ZrtpPacketClearAck.h>
#include <libzrtpcpp/ZrtpPacketError.h>
#include <libzrtpcpp/ZrtpPacketErrorAck.h>
#include <libzrtpcpp/ZrtpPacketPing.h>
#include <libzrtpcpp/ZrtpPacketPingAck.h>
#include <libzrtpcpp/ZrtpCallback.h>
#include <libzrtpcpp/ZIDRecord.h>

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32
#endif

class ZrtpStateClass;
class ZrtpDH;

/**
 * The main ZRTP class.
 *
 * This is the main class of the RTP/SRTP independent part of the GNU
 * ZRTP. It handles the ZRTP HMAC, DH, and other data management. The
 * user of this class needs to know only a few methods and needs to
 * provide only a few external functions to connect to a Timer
 * mechanism and to send data via RTP and SRTP. Refer to the
 * ZrtpCallback class to get detailed information regading the
 * callback methods required by GNU RTP.
 *
 * The class ZrtpQueue is the GNU ccRTP specific implementation that
 * extends standard ccRTP RTP provide ZRTP support. Refer to the
 * documentation of ZrtpQueue to get more information about the usage
 * of ZRtp and associated classes.
 *
 * The main entry into the ZRTP class is the processExtensionHeader()
 * method.
 *
 * This class does not directly handle the protocol states, timers,
 * and packet resend. The protocol state engine is responsible for
 * these actions.
 * 
 * Example how to use ZRtp:
 *<pre>
 *    zrtpEngine = new ZRtp((uint8_t*)ownZid, (ZrtpCallback*)this, idString);
 *    zrtpEngine->startZrtpEngine();
 *</pre>
 * @see ZrtpCallback
 *
 * @author Werner Dittmann <Werner.Dittmann@t-online.de>
 */
class ZRtp {

    public:

    /**
     * Constructor intializes all relevant data but does not start the
     * engine.
     */
    ZRtp(uint8_t* myZid, ZrtpCallback* cb, std::string id);

    /**
     * Destructor cleans up.
     */
    ~ZRtp();

    /**
     * Kick off the ZRTP protocol engine.
     *
     * This method calls the ZrtpStateClass#evInitial() state of the state
     * engine. After this call we are able to process ZRTP packets
     * from our peer and to process them.
     */
    void startZrtpEngine();

    /**
     * Stop ZRTP security.
     *
     */
    void stopZrtp();

    /**
     * Process RTP extension header.
     *
     * This method expects to get a pointer to the extension header of
     * a RTP packet. The method checks if this is really a ZRTP
     * packet. If this check fails the method returns 0 (false) in
     * case this is not a ZRTP packet. We return a 1 if we processed
     * the ZRTP extension header and the caller may process RTP data
     * after the extension header as usual.  The method return -1 the
     * call shall dismiss the packet and shall not forward it to
     * further RTP processing.
     *
     * @param extHeader
     *    A pointer to the first byte of the extension header. Refer to
     *    RFC3550.
     * @param peerSSRC
     *    The peer's SSRC.
     * @return
     *    Code indicating further packet handling, see description above.
     */
    void processZrtpMessage(uint8_t *extHeader, uint32_t peerSSRC);

    /**
     * Process a timeout event.
     *
     * We got a timeout from the timeout provider. Forward it to the
     * protocol state engine.
     *
     */
    void processTimeout();

    /**
     * Check for and handle GoClear ZRTP packet header.
     *
     * This method checks if this is a GoClear packet. If not, just return
     * false. Otherwise handle it according to the specification.
     *
     * @param extHeader
     *    A pointer to the first byte of the extension header. Refer to
     *    RFC3550.
     * @return
     *    False if not a GoClear, true otherwise.
     */
    bool handleGoClear(uint8_t *extHeader);

    /**
     * Set the auxilliary secret.
     *
     * Use this method to set the auxilliary secret data. Refer to ZRTP
     * specification, chapter 4.3 ff
     *
     * @param data
     *     Points to the srtps secret data.
     * @param length
     *     Length of the auxilliary secrect in bytes
     */
    void setAuxSecret(uint8_t* data, int32_t length);

    /**
     * Set the PBX secret.
     *
     * Use this method to set the PBX secret data. Refer to ZRTP
     * specification, chapter 4.3 ff and 7.3
     *
     * @param data
     *     Points to the other secret data.
     * @param length
     *     The length in bytes of the data.
     */
    void setPbxSecret(uint8_t* data, int32_t length);

    /**
     * Check current state of the ZRTP state engine
     *
     * @param state
     *    The state to check.
     * @return
     *    Returns true id ZRTP engine is in the given state, false otherwise.
     */
    bool inState(int32_t state);

    /**
     * Set SAS as verified.
     *
     * Call this method if the user confirmed (verfied) the SAS. ZRTP
     * remembers this together with the retained secrets data.
     */
    void SASVerified();

    /**
     * Reset the SAS verfied flag for the current active user's retained secrets.
     *
     */
    void resetSASVerified();

   /**
    * Get the ZRTP Hello Hash data.
    *
    * Use this method to get the ZRTP Hello Hash data. The method 
    * returns the data as a string containing the ZRTP protocol version and
    * hex-digits. Refer to ZRTP specification, chapter 8.
    *
    * @return
    *    a std:string containing the Hello hash value as hex-digits. The
    *    hello hash is available immediately after class instantiation.
    */
   std::string getHelloHash();

    /**
     * Get Multi-stream parameters.
     *
     * Use this method to get the Multi-stream that were computed during
     * the ZRTP handshake. An application may use these parameters to
     * enable multi-stream processing for an associated SRTP session.
     *
     * Refer to chapter 4.4.2 in the ZRTP specification for further details
     * and restriction how and when to use multi-stream mode.
     *
     * @return
     *    a string that contains the multi-stream parameters. The application
     *    must not modify the contents of this string, it is opaque data. The
     *    application may hand over this string to a new ZrtpQueue instance
     *    to enable multi-stream processing for this ZrtpQueue.
     *    If ZRTP was not started or ZRTP is not yet in secure state the method
     *    returns an empty string.
     */
    std::string getMultiStrParams();

    /**
     * Set Multi-stream parameters.
     *
     * Use this method to set the parameters required to enable Multi-stream
     * processing of ZRTP. The multi-stream parameters must be set before the
     * application starts the ZRTP protocol engine.
     *
     * Refer to chapter 4.4.2 in the ZRTP specification for further details
     * of multi-stream mode.
     *
     * @param parameters
     *     A string that contains the multi-stream parameters that this
     *     new ZrtpQueue instanace shall use. See also 
     *     <code>getMultiStrParams()</code>
     */
    void setMultiStrParams(std::string parameters);

    /**
     * Check if this ZRTP use Multi-stream.
     *
     * Use this method to check if this ZRTP instance uses multi-stream. 
     * Refer to chapters 4.2 and 4.4.2 in the ZRTP.
     *
     * @return
     *     True if multi-stream is used, false otherwise.
     */
    bool isMultiStream();

    /**
     * Accept a PBX enrollment request.
     *
     * If a PBX service asks to enroll the PBX trusted MitM key and the user 
     * accepts this request, for example by pressing an OK button, the client
     * application shall call this method and set the parameter 
     * <code>accepted</code> to true. If the user does not accept the request 
     * set the parameter to false.
     *
     * @param accepted
     *     True if the enrollment request is accepted, false otherwise.
     */
    void acceptEnrollment(bool accepted);

    /**
     * Enable PBX enrollment
     *
     * The application calls this method to allow or disallow PBX enrollment.
     * If the applications allows PBX enrollment then the ZRTP implementation
     * honors the PBX enrollment flag in Confirm packets. Refer to chapter 7.3
     * for further details of PBX enrollment.
     *
     * @param yesNo
     *    If set to true then ZRTP honors the PBX enrollment flag in Commit
     *    packets and calls the appropriate user callback methods. If
     *    the parameter is set to false ZRTP ignores the PBX enrollment flags.
     */
    void setPBXEnrollment(bool yesNo);

    /**
     * Set signature data
     *
     * This functions stores signature data and transmitts it during ZRTP
     * processing to the other party as part of the Confirm packets. Refer to 
     * chapters 5.7 and 7.2.
     *
     * The signature data must be set before ZRTP the application calls
     * <code>start()</code>.
     *
     * @param data
     *    The signature data including the signature type block. The method
     *    copies this data into the Confirm packet at signature type block.
     * @param length
     *    The length of the signature data in bytes. This length must be
     *    multiple of 4.
     * @return
     *    True if the method stored the data, false otherwise.
     */
    bool setSignatureData(uint8_t* data, int32_t length);

    /**
     * Get signature data
     *
     * This functions returns signature data that was receivied during ZRTP
     * processing. Refer to chapters 5.7 and 7.2.
     *
     * The signature data can be retrieved after ZRTP enters secure state.
     * <code>start()</code>.
     *
     * @param data
     *    Pointer to a data buffer. This buffer must be large enough to
     *    hold the signature data. Refer to <code>getSignatureLength()</code>
     *    to get the length of the received signature data.
     * @return
     *    Number of bytes copied into the data buffer
     */
    int32_t getSignatureData(uint8_t* data);

    /**
     * Get length of signature data
     *
     * This functions returns the length of signature data that was receivied 
     * during ZRTP processing. Refer to chapters 5.7 and 7.2.
     *
     * @return
     *    Length in bytes of the received signature data. The method returns
     *    zero if no signature data avilable.
     */
    int32_t getSignatureLength();

    /**
     * Emulate a Conf2Ack packet.
     *
     * This method emulates a Conf2Ack packet. According to ZRTP specification
     * the first valid SRTP packet that the Initiator receives must switch 
     * on secure mode. Refer to chapter 4 in the specificaton
     *
     */
    void conf2AckSecure();

     /**
      * Get other party's ZID (ZRTP Identifier) data
      *
      * This functions returns the other party's ZID that was receivied 
      * during ZRTP processing. 
      *
      * The ZID data can be retrieved after ZRTP receive the first Hello
      * packet from the other party. The application may call this method
      * for example during SAS processing in showSAS(...) user callback
      * method.
      *
      * @param data
      *    Pointer to a data buffer. This buffer must have a size of
      *    at least 12 bytes (96 bit) (ZRTP Identifier, see chap. 4.9)
      * @return
      *    Number of bytes copied into the data buffer - must be equivalent
      *    to 96 bit, usually 12 bytes.
      */
     int32_t getZid(uint8_t* data);

private:
     friend class ZrtpStateClass;

    /**
     * The state engine takes care of protocol processing.
     */
    ZrtpStateClass* stateEngine;

    /**
     * This is my ZID that I send to the peer.
     */
    uint8_t zid[IDENTIFIER_LEN];

    /**
     * The peer's ZID
     */
    uint8_t peerZid[IDENTIFIER_LEN];

    /**
     * The callback class provides me with the interface to send
     * data and to deal with timer management of the hosting system.
     */
    ZrtpCallback* callback;

    /**
     * My active Diffie-Helman context
     */
    ZrtpDH* dhContext;

    /**
     * The computed DH shared secret
     */
    uint8_t* DHss;

    /**
     * My computed public key
     */
    uint8_t pubKeyBytes[400];
    /**
     * Length off public key
     */
    int32_t pubKeyLen;
    /**
     * My Role in the game
     */
    Role myRole;

    /**
     * The human readable SAS value
     */
    std::string SAS;

    /**
     * The SAS hash for signaling and alike. Refer to chapters
     * 4.5 and 7 how sasHash, sasValue and the SAS string are derived.
     */
    uint8_t sasHash[SHA256_DIGEST_LENGTH];
    /**
     * The ids for the retained and other shared secrets
     */
    uint8_t rs1IDr[SHA256_DIGEST_LENGTH];
    uint8_t rs2IDr[SHA256_DIGEST_LENGTH];
    uint8_t auxSecretIDr[SHA256_DIGEST_LENGTH];
    uint8_t pbxSecretIDr[SHA256_DIGEST_LENGTH];

    uint8_t rs1IDi[SHA256_DIGEST_LENGTH];
    uint8_t rs2IDi[SHA256_DIGEST_LENGTH];
    uint8_t auxSecretIDi[SHA256_DIGEST_LENGTH];
    uint8_t pbxSecretIDi[SHA256_DIGEST_LENGTH];

    /**
     * pointers to aux secret storage and length of aux secret
     */
    uint8_t* auxSecret;
    int32_t auxSecretLength;

    /**
     * Record if valid rs1 and/or rs1 were found in the 
     * retaind secret cache.
     */
    bool rs1Valid;
    bool rs2Valid;
    /**
     * My hvi
     */
    uint8_t hvi[SHA256_DIGEST_LENGTH];

    /**
     * The peer's hvi
     */
    uint8_t peerHvi[SHA256_DIGEST_LENGTH];

    /**
     * Context to compute the SHA256 hash of selected messages.
     * Used to compute the s0, refer to chapter 4.4.1.4
     */
    void* msgShaContext;
    /**
     * Commited Hash, Cipher, and public key algorithms
     */
    SupportedHashes hash;
    SupportedSymCiphers cipher;
    SupportedPubKeys pubKey;
    /**
     * The selected SAS type.
     */
    SupportedSASTypes sasType;

    /**
     * The selected SAS type.
     */
    SupportedAuthLengths authLength;

    /**
     * The Hash images as defined in chapter 5.1.1 (H0 is a random value,
     * not stored here). Need full SHA 256 lenght to store hash value but
     * only the leftmost 128 bits are used in computations and comparisons.
     */
    uint8_t H0[SHA256_DIGEST_LENGTH];
    uint8_t H1[SHA256_DIGEST_LENGTH];
    uint8_t H2[SHA256_DIGEST_LENGTH];
    uint8_t H3[SHA256_DIGEST_LENGTH];
    uint8_t helloHash[SHA256_DIGEST_LENGTH];

    uint8_t peerH0[SHA256_DIGEST_LENGTH];
    uint8_t peerH1[SHA256_DIGEST_LENGTH];
    uint8_t peerH2[SHA256_DIGEST_LENGTH];
    uint8_t peerH3[SHA256_DIGEST_LENGTH];

    /**
     * The SHA256 hash over selected messages
     */
    uint8_t messageHash[SHA256_DIGEST_LENGTH];
    /**
     * The s0
     */
    uint8_t s0[SHA256_DIGEST_LENGTH];

    /**
     * The new Retained Secret
     */
    uint8_t newRs1[RS_LENGTH];

    /**
     * The GoClear HMAC keys and confirm HMAC key
     */
    uint8_t hmacKeyI[SHA256_DIGEST_LENGTH];
    uint8_t hmacKeyR[SHA256_DIGEST_LENGTH];

    /**
     * The Initiator's srtp key and salt
     */
    uint8_t srtpKeyI[SHA256_DIGEST_LENGTH];
    uint8_t srtpSaltI[SHA256_DIGEST_LENGTH];

    /**
     * The Responder's srtp key and salt
     */
    uint8_t srtpKeyR[SHA256_DIGEST_LENGTH];
    uint8_t srtpSaltR[SHA256_DIGEST_LENGTH];

    /**
     * The keys used to encrypt/decrypt the confirm message
     */
    uint8_t zrtpKeyI[SHA256_DIGEST_LENGTH];
    uint8_t zrtpKeyR[SHA256_DIGEST_LENGTH];

    /**
     * The ZRTP Session Key
     * Refer to chapter 5.4.1.4
     */
    uint8_t zrtpSession[SHA256_DIGEST_LENGTH];

    /**
     * True if this ZRTP instance uses multi-stream mode.
     */
    bool multiStream;

    /**
     * True if PBX enrollment is enabled.
     */
    bool PBXEnrollment;

    /**
     * Pre-initialized packets.
     */
    ZrtpPacketHello    zrtpHello;
    ZrtpPacketHelloAck zrtpHelloAck;
    ZrtpPacketConf2Ack zrtpConf2Ack;
    ZrtpPacketClearAck zrtpClearAck;
    ZrtpPacketGoClear  zrtpGoClear;
    ZrtpPacketError    zrtpError;
    ZrtpPacketErrorAck zrtpErrorAck;
    ZrtpPacketDHPart   zrtpDH1;
    ZrtpPacketDHPart   zrtpDH2;
    ZrtpPacketCommit   zrtpCommit;
    ZrtpPacketConfirm  zrtpConfirm1;
    ZrtpPacketConfirm  zrtpConfirm2;
    ZrtpPacketPingAck  zrtpPingAck;

    /**
     * Random IV data to encrypt the confirm data, 128 bit for AES
     */
    uint8_t randomIV[16];

    uint8_t tempMsgBuffer[1024];
    int32_t lengthOfMsgData;

    /**
     * Variables to store signature data. Includes the signature type block
     */
    uint8_t* signatureData;       // will be allocated when needed
    int32_t  signatureLength;     // overall length in bytes

    uint32_t peerSSRC;            // peer's SSRC, required to setup PingAck packet
    /**
     * Find the best Hash algorithm that is offered in Hello.
     *
     * Find the best, that is the strongest, Hash algorithm that our peer
     * offers in its Hello packet.
     *
     * @param hello
     *    The Hello packet.
     * @return
     *    The Enum that identifies the best offered Hash algortihm. Return
     *    <code>NumSupportedHashes</code> to signal that no matching Hash algorithm
     *     was found at all.
    */
    SupportedHashes findBestHash(ZrtpPacketHello *hello);

    /**
     * Find the best symmetric cipher algorithm that is offered in Hello.
     *
     * Find the best, that is the strongest, cipher algorithm that our peer
     * offers in its Hello packet.
     *
     * @param hello
     *    The Hello packet.
     * @param pk
     *    The id of the selected public key algorithm 
     * @return
     *    The Enum that identifies the best offered Cipher algortihm. Return
     *    <code>NumSupportedSymCiphers</code> to signal that no matching Cipher algorithm
     *    was found at all.
     */
    SupportedSymCiphers findBestCipher(ZrtpPacketHello *hello,  SupportedPubKeys pk);

    /**
     * Find the best Public Key algorithm that is offered in Hello.
     *
     * Find the best, that is the strongest, public key algorithm that our peer
     * offers in its Hello packet.
     *
     * @param hello
     *    The Hello packet.
     * @return
     *    The Enum that identifies the best offered Public Key algortihm. Return
     *    <code>NumSupportedPubKeys</code> to signal that no matching Public Key algorithm
     *    was found at all.
     */
    SupportedPubKeys findBestPubkey(ZrtpPacketHello *hello);

    /**
     * Find the best SAS algorithm that is offered in Hello.
     *
     * Find the best, that is the strongest, SAS algorithm that our peer
     * offers in its Hello packet.
     *
     * @param hello
     *    The Hello packet.
     * @return
     *    The Enum that identifies the best offered SAS algortihm. Return
     *    <code>NumSupportedSASTypes</code> to signal that no matching SAS algorithm
     *    was found at all.
     */
    SupportedSASTypes findBestSASType(ZrtpPacketHello* hello);

    /**
     * Find the best authentication length that is offered in Hello.
     *
     * Find the best, that is the strongest, authentication length that our peer
     * offers in its Hello packet.
     *
     * @param hello
     *    The Hello packet.
     * @return
     *    The Enum that identifies the best offered authentication length. Return
     *    <code>NumSupportedAuthLenghts</code> to signal that no matching length
     *    was found at all.
     */
    SupportedAuthLengths findBestAuthLen(ZrtpPacketHello* hello);

    /**
     * Check if MultiStream mode is offered in Hello.
     *
     * Find the best, that is the strongest, authentication length that our peer
     * offers in its Hello packet.
     *
     * @param hello
     *    The Hello packet.
     * @return
     *    True if multi stream mode is available, false otherwise.
     */
    bool checkMultiStream(ZrtpPacketHello* hello);

    /**
     * Compute my hvi value according to ZRTP specification.
     */
    void computeHvi(ZrtpPacketDHPart* dh, ZrtpPacketHello *hello);

    void computeSharedSecretSet(ZIDRecord& zidRec);

    void computeSRTPKeys();

    void generateKeysInitiator(ZrtpPacketDHPart *dhPart, ZIDRecord& zidRec);

    void generateKeysResponder(ZrtpPacketDHPart *dhPart, ZIDRecord& zidRec);

    void generateKeysMultiStream();

    /*
     * The following methods are helper functions for ZrtpStateClass.
     * ZrtpStateClass calls them to prepare packets, send data, report
     * problems, etc.
     */
    /**
     * Send a ZRTP packet.
     *
     * The state engines calls this method to send a packet via the RTP
     * stack.
     *
     * @param packet
     *    Points to the ZRTP packet.
     * @return
     *    zero if sending failed, one if packet was send
     */
    int32_t sendPacketZRTP(ZrtpPacketBase *packet);

    /**
     * Activate a Timer using the host callback.
     *
     * @param tm
     *    The time in milliseconds.
     * @return
     *    zero if activation failed, one if timer was activated
     */
    int32_t activateTimer(int32_t tm);

    /**
     * Cancel the active Timer using the host callback.
     *
     * @return
     *    zero if activation failed, one if timer was activated
     */
    int32_t cancelTimer();

    /**
     * Prepare a Hello packet.
     *
     * Just take the preinitialized Hello packet and return it. No
     * further processing required.
     *
     * @return
     *    A pointer to the initialized Hello packet.
     */
    ZrtpPacketHello* prepareHello();

    /**
     * Prepare a HelloAck packet.
     *
     * Just take the preinitialized HelloAck packet and return it. No
     * further processing required.
     *
     * @return
     *    A pointer to the initialized HelloAck packet.
     */
    ZrtpPacketHelloAck* prepareHelloAck();

    /**
     * Prepare a Commit packet.
     *
     * We have received a Hello packet from our peer. Check the offers
     * it makes to us and select the most appropriate. Using the
     * selected values prepare a Commit packet and return it to protocol
     * state engine.
     *
     * @param hello
     *    Points to the received Hello packet
     * @param errMsg
     *    Points to an integer that can hold a ZRTP error code.
     * @return
     *    A pointer to the prepared Commit packet
     */
    ZrtpPacketCommit* prepareCommit(ZrtpPacketHello *hello, uint32_t* errMsg);

    /**
     * Prepare a Commit packet for Multi Stream mode.
     *
     * Using the selected values prepare a Commit packet and return it to protocol
     * state engine.
     *
     * @param hello
     *    Points to the received Hello packet
     * @return
     *    A pointer to the prepared Commit packet for multi stream mode
     */
    ZrtpPacketCommit* prepareCommitMultiStream(ZrtpPacketHello *hello);

    /**
     * Prepare the DHPart1 packet.
     *
     * This method prepares a DHPart1 packet. The input to the method is always
     * a Commit packet received from the peer. Also we a in the role of the
     * Responder.
     *
     * When we receive a Commit packet we get the selected ciphers, hashes, etc
     * and cross-check if this is ok. Then we need to initialize a set of DH
     * keys according to the selected cipher. Using this data we prepare our DHPart1
     * packet.
     */
    ZrtpPacketDHPart* prepareDHPart1(ZrtpPacketCommit *commit, uint32_t* errMsg);

    /**
     * Prepare the DHPart2 packet.
     *
     * This method prepares a DHPart2 packet. The input to the method is always
     * a DHPart1 packet received from the peer. Our peer sends the DH1Part as
     * response to our Commit packet. Thus we are in the role of the
     * Initiator.
     *
     */
    ZrtpPacketDHPart* prepareDHPart2(ZrtpPacketDHPart* dhPart1, uint32_t* errMsg);

    /**
     * Prepare the Confirm1 packet.
     *
     * This method prepare the Confirm1 packet. The input to this method is the
     * DHPart2 packect received from our peer. The peer sends the DHPart2 packet
     * as response of our DHPart1. Here we are in the role of the Responder
     *
     */
    ZrtpPacketConfirm* prepareConfirm1(ZrtpPacketDHPart* dhPart2, uint32_t* errMsg);

    /**
     * Prepare the Confirm1 packet in multi stream mode.
     *
     * This method prepares the Confirm1 packet. The state engine call this method 
     * if multi stream mode is selected and a Commit packet was received. The input to 
     * this method is the Commit.
     * Here we are in the role of the Responder
     *
     */
    ZrtpPacketConfirm* prepareConfirm1MultiStream(ZrtpPacketCommit* commit, uint32_t* errMsg);

    /**
     * Prepare the Confirm2 packet.
     *
     * This method prepare the Confirm2 packet. The input to this method is the
     * Confirm1 packet received from our peer. The peer sends the Confirm1 packet
     * as response of our DHPart2. Here we are in the role of the Initiator
     */
    ZrtpPacketConfirm* prepareConfirm2(ZrtpPacketConfirm* confirm1, uint32_t* errMsg);

    /**
     * Prepare the Confirm2 packet in multi stream mode.
     *
     * This method prepares the Confirm2 packet. The state engine call this method if
     * multi stream mode is active and in state CommitSent. The input to this method is 
     * the Confirm1 packet received from our peer. The peer sends the Confirm1 packet
     * as response of our Commit packet in multi stream mode. 
     * Here we are in the role of the Initiator
     */
    ZrtpPacketConfirm* prepareConfirm2MultiStream(ZrtpPacketConfirm* confirm1, uint32_t* errMsg);

    /**
     * Prepare the Conf2Ack packet.
     *
     * This method prepare the Conf2Ack packet. The input to this method is the
     * Confirm2 packet received from our peer. The peer sends the Confirm2 packet
     * as response of our Confirm1. Here we are in the role of the Initiator
     */
    ZrtpPacketConf2Ack* prepareConf2Ack(ZrtpPacketConfirm* confirm2, uint32_t* errMsg);

    /**
     * Prepare the ErrorAck packet.
     *
     * This method prepares the ErrorAck packet. The input to this method is the
     * Error packet received from the peer.
     */
    ZrtpPacketErrorAck* prepareErrorAck(ZrtpPacketError* epkt);

    /**
     * Prepare the Error packet.
     *
     * This method prepares the Error packet. The input to this method is the
     * error code to be included into the message.
     */
    ZrtpPacketError* prepareError(uint32_t errMsg);

    /**
     * Prepare a ClearAck packet.
     *
     * This method checks if the GoClear message is valid. If yes then switch
     * off SRTP processing, stop sending of RTP packets (pause transmit) and
     * inform the user about the fact. Only if user confirms the GoClear message
     * normal RTP processing is resumed.
     *
     * @return
     *     NULL if GoClear could not be authenticated, a ClearAck packet
     *     otherwise.
     */
    ZrtpPacketClearAck* prepareClearAck(ZrtpPacketGoClear* gpkt);

    /**
     * Prepare the ErrorAck packet.
     *
     * This method prepares the ErrorAck packet. The input to this method is the
     * Error packet received from the peer.
     */
    ZrtpPacketPingAck* preparePingAck(ZrtpPacketPing* ppkt);

    /**
     * Prepare a GoClearAck packet w/o HMAC
     *
     * Prepare a GoCLear packet without a HMAC but with a short error message.
     * This type of GoClear is used if something went wrong during the ZRTP
     * negotiation phase.
     *
     * @return
     *     A goClear packet without HMAC
     */
    ZrtpPacketGoClear* prepareGoClear(uint32_t errMsg = 0);

    /**
     * Compare the hvi values.
     *
     * Compare a received Commit packet with our Commit packet and returns
     * which Commit packt is "more important". See chapter 5.2 to get further
     * information how to compare Commit packets.
     *
     * @param commit
     *    Pointer to the peer's commit packet we just received.
     * @return
     *    <0 if our Commit packet is "less important"
     *    >0 if our Commit is "more important"
     *     0 shouldn't happen because we compare crypto hashes
     */
    int32_t compareCommit(ZrtpPacketCommit *commit);

    /**
     * Verify the H2 hash image.
     *
     * Verifies the H2 hash contained in a received commit message.
     * This functions just verifies H2 but does not store it.
     *
     * @param commit
     *    Pointer to the peer's commit packet we just received.
     * @return
     *    true if H2 is ok and verified
     *    false if H2 could not be verified
     */
    bool verifyH2(ZrtpPacketCommit *commit);

    /**
     * Send information messages to the hosting environment.
     *
     * The ZRTP implementation uses this method to send information messages
     * to the host. Along with the message ZRTP provides a severity indicator
     * that defines: Info, Warning, Error, Alert. Refer to the MessageSeverity
     * enum in the ZrtpCallback class.
     *
     * @param severity
     *     This defines the message's severity
     * @param subCode
     *     The subcode identifying the reason.
     * @see ZrtpCodes#MessageSeverity
     */
    void sendInfo(GnuZrtpCodes::MessageSeverity severity, int32_t subCode);

    /**
     * ZRTP state engine calls this if the negotiation failed.
     *
     * ZRTP calls this method in case ZRTP negotiation failed. The parameters
     * show the severity as well as some explanatory text.
     *
     * @param severity
     *     This defines the message's severity
     * @param subCode
     *     The subcode identifying the reason.
     * @see ZrtpCodes#MessageSeverity
     */
    void zrtpNegotiationFailed(GnuZrtpCodes::MessageSeverity severity, int32_t subCode);

    /**
     * ZRTP state engine calls this method if the other side does not support ZRTP.
     *
     * If the other side does not answer the ZRTP <em>Hello</em> packets then
     * ZRTP calls this method,
     *
     */
    void zrtpNotSuppOther();

    /**
     * Signal SRTP secrets are ready.
     *
     * This method calls a callback method to inform the host that the SRTP
     * secrets are ready.
     *
     * @param part
     *    Defines for which part (sender or receiver) to switch on security
     * @return
     *    Returns false if something went wrong during initialization of SRTP
     *    context. Propagate error back to state engine.
     */
    bool srtpSecretsReady(EnableSecurity part);

    /**
     * Switch off SRTP secrets.
     *
     * This method calls a callback method to inform the host that the SRTP
     * secrets shall be cleared.
     *
     * @param part
     *    Defines for which part (sender or receiver) to clear
     */
    void srtpSecretsOff(EnableSecurity part);

    /**
     * ZRTP state engine calls these methods to enter or leave its 
     * synchronization mutex.
     */
    void synchEnter();

    void synchLeave();

    /**
     * Helper function to store ZRTP message data in a temporary buffer
     *
     * This functions first clears the temporary buffer, then stores
     * the packet's data to it. We use this to check the packet's HMAC
     * after we received the HMAC key in to following packet.
     *
     * @param data
     *    Pointer to the packet's ZRTP message
    */
     void storeMsgTemp(ZrtpPacketBase* pkt);

     /**
      * Helper function to check a ZRTP message HMAC
      *
      * This function gets a HMAC key and uses it to compute a HMAC
      * with this key and the stored data of a previous received ZRTP
      * message. It compares the computed HMAC and the HMAC stored in
      * the received message and returns the result.
      *
      * @param key
      *    Pointer to the HMAC key.
      * @return
      *    Returns true if the computed HMAC and the stored HMAC match,
      *    false otherwise.
      */
     bool checkMsgHmac(uint8_t* key);

     /**
      * Set the client ID for ZRTP Hello message.
      *
      * The user of ZRTP must set its id to identify itself in the
      * ZRTP HELLO message. The maximum length is 16 characters. Shorter
      * id string are allowed, they will be filled with blanks. A longer id
      * is truncated to 16 characters.
      *
      * The identifier is set in the Hello packet of ZRTP. Thus only after
      * setting the identifier ZRTP can compute the HMAC and the final
      * helloHash. 
      *
      * @param id
      *     The client's id
      */
     void setClientId(std::string id);
};

#endif // ZRTP
