/*******************************************************************************
*
*               COPYRIGHT (c) 2015-2016 Broadlink Corporation
*                         All Rights Reserved
*
* The source code contained or described herein and all documents
* related to the source code ("Material") are owned by Broadlink
* Corporation or its licensors.  Title to the Material remains
* with Broadlink Corporation or its suppliers and licensors.
*
* The Material is protected by worldwide copyright and trade secret
* laws and treaty provisions. No part of the Material may be used,
* copied, reproduced, modified, published, uploaded, posted, transmitted,
* distributed, or disclosed in any way except in accordance with the
* applicable license agreement.
*
* No license under any patent, copyright, trade secret or other
* intellectual property right is granted to or conferred upon you by
* disclosure or delivery of the Materials, either expressly, by
* implication, inducement, estoppel, except in accordance with the
* applicable license agreement.
*
* Unless otherwise agreed by Broadlink in writing, you may not remove or
* alter this notice or any other notice embedded in Materials by Broadlink
* or Broadlink's suppliers or licensors in any way.
*
*******************************************************************************/

#ifndef __DNA_WLAN_H
#define __DNA_WLAN_H

#include "dna_compiler.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define DNA_WLAN_PROBE_RESPONSE_RESERVED_LEN         12

/** @defgroup dna_wlan_enum Enum
  * @{
  */

typedef enum {
    DNA_WLAN_SEC_OPEN = 0,
    DNA_WLAN_SEC_WEP,
    DNA_WLAN_SEC_TKIP,
    DNA_WLAN_SEC_AES,
    DNA_WLAN_SEC_AES_TKIP,
    DNA_WLAN_SEC_UNKNOWN,
} dna_wlan_sec_type_e;

typedef enum {
    /* Get IP success */
    DNA_WLAN_STA_CONNECTED = 1,
} dna_wlan_sta_status_e;

typedef enum {
    DNA_WLAN_AP_STARTED = 1,
} dna_wlan_ap_status_e;

typedef enum {
    DNA_WLAN_RADIO_OFF = 0,
    DNA_WLAN_RADIO_ON,
} dna_wlan_rf_type_e;

typedef enum {
    DNA_WLAN_SNIFFER_PKT_BEACON = 0,
    DNA_WLAN_SNIFFER_PKT_PROBE_REQ,
    DNA_WLAN_SNIFFER_PKT_PROBE_RSP,
    DNA_WLAN_SNIFFER_PKT_BC_DATA,
    DNA_WLAN_SNIFFER_PKT_MC_DATA,
} dna_wlan_sniffer_filter_e;

#define DNA_WLAN_SNIFFER_FILTER_BEACON      (BIT(DNA_WLAN_SNIFFER_PKT_BEACON))
#define DNA_WLAN_SNIFFER_FILTER_PROBE_REQ   (BIT(DNA_WLAN_SNIFFER_PKT_PROBE_REQ))
#define DNA_WLAN_SNIFFER_FILTER_PROBE_RSP   (BIT(DNA_WLAN_SNIFFER_PKT_PROBE_RSP))
#define DNA_WLAN_SNIFFER_FILTER_PROBE       (DNA_WLAN_SNIFFER_FILTER_PROBE_REQ | \
                                             DNA_WLAN_SNIFFER_FILTER_PROBE_RSP)
#define DNA_WLAN_SNIFFER_FILTER_DATA        (BIT(DNA_WLAN_SNIFFER_PKT_BC_DATA) | \
                                             BIT(DNA_WLAN_SNIFFER_PKT_MC_DATA))
#define DNA_WLAN_SNIFFER_FILTER_ALL         (DNA_WLAN_SNIFFER_FILTER_BEACON | \
                                             DNA_WLAN_SNIFFER_FILTER_PROBE  | \
                                             DNA_WLAN_SNIFFER_FILTER_DATA)

#define DNA_WLAN_MGMT_PKT_FILTER_ASSOC_REQ       BIT(0)
#define DNA_WLAN_MGMT_PKT_FILTER_ASSOC_RESP      BIT(1)
#define DNA_WLAN_MGMT_PKT_FILTER_REASSOC_REQ     BIT(2)
#define DNA_WLAN_MGMT_PKT_FILTER_REASSOC_RESP    BIT(3)
#define DNA_WLAN_MGMT_PKT_FILTER_PROBE_REQ       BIT(4)
#define DNA_WLAN_MGMT_PKT_FILTER_PROBE_RESP      BIT(5)
#define DNA_WLAN_MGMT_PKT_FILTER_BEACON          BIT(8)
#define DNA_WLAN_MGMT_PKT_FILTER_ATIM            BIT(9)
#define DNA_WLAN_MGMT_PKT_FILTER_REASSOC         BIT(10)
#define DNA_WLAN_MGMT_PKT_FILTER_AUTH            BIT(11)
#define DNA_WLAN_MGMT_PKT_FILTER_DEAUTH          BIT(12)

/**
  * @}
  */

/* WLAN 802.11 (MAC) standard header */
typedef DNA_PACKED_START struct dna_wlan_mac_hdr {
    struct {
        unsigned short ver:2;
        unsigned short type:2;
        unsigned short subtype:4;
        unsigned short tods:1;
        unsigned short frds:1;
        unsigned short morefrag:1;
        unsigned short retry:1;
        unsigned short pwrmgmt:1;
        unsigned short moredata:1;
        unsigned short wep:1;
        unsigned short order:1;
    }fc;
    unsigned short duration;
    unsigned char addr1[6];
    unsigned char addr2[6];
    unsigned char addr3[6];
    unsigned short frag:4;
    unsigned short sequence:12;
} DNA_PACKED_END dna_wlan_mac_hdr_t;

typedef void (* dna_wlan_sniffer_handler_t)(
    /* Frame point to 802.11 header (Please refer to 802.11 protocol) */
    const void * frame,
    /* Sniffer packet length */
    const unsigned short len
);

typedef void (* dna_wlan_extend_sniffer_handler_t)(
	signed char rssi,
    /* Frame point to 802.11 header (Please refer to 802.11 protocol) */
    const void * frame,
    /* Sniffer packet length */
    const unsigned short len
);

typedef struct {
    unsigned char element_id;
    unsigned char length;
    unsigned char OUI[3];
} dna_wlan_vender_ie_t;

typedef struct dna_wifi_network_old {
    unsigned int ipaddr;
    unsigned int mask;
    unsigned int gateway;
    unsigned int dns1;
    unsigned int dns2;
    unsigned char ssid[32];
    unsigned char psk[32];
    unsigned char ssid_len;
    unsigned char psk_len;
    /* Refer to dna_wlan_sec_type_e */
    unsigned char sec_type;
    unsigned char channel;
    unsigned char bssid[6];
    unsigned char reserved[2];
} dna_wifi_network_old_t;

typedef struct dna_wifi_network {
    unsigned int ipaddr;
    unsigned int mask;
    unsigned int gateway;
    unsigned int dns1;
    unsigned int dns2;
    unsigned char ssid[33];
    unsigned char psk[65];
    unsigned char ssid_len;
    unsigned char psk_len;
    /* Refer to dna_wlan_sec_type_e */
    unsigned char sec_type;
    unsigned char channel;
    unsigned char bssid[6];
} dna_wifi_network_t;

typedef struct dna_ipconfig {
    unsigned int ip;
    unsigned int gw;
    unsigned int netmask;
} dna_ipconfig_t;

typedef struct dna_wlan_scan_result {
    /** The network SSID, represented as a NULL-terminated C string of 0 to 32
      *  characters.  If the network has a hidden SSID, this will be the empty
      *  string.
      */
    char ssid[33];
    /** SSID length */
    unsigned int ssid_len;
    /** The network BSSID, represented as a 6-byte array. */
    char bssid[6];
    /** The network channel. */
    unsigned int channel;
    /** The network wireless mode. */
    unsigned char mode;

    /* network features */
    /** The network supports WMM.  This is set to 0 if the network does not
      *  support WMM or if the system does not have WMM support enabled. */
    unsigned wmm:1;
    /** The network supports WPS.  This is set to 0 if the network does not
      *  support WPS or if the system does not have WPS support enabled. */
    unsigned wps:1;
    /** WPS Type PBC/PIN */
    unsigned int wps_session;
#ifdef CONFIG_WPA2_ENTP
    /** WPA2 Enterprise security */
    unsigned wpa2_entp:1;
#endif
    /** The network uses WEP security. */
    unsigned wep:1;
    /** The network uses WPA security. */
    unsigned wpa:1;
    /** The network uses WPA2 security */
    unsigned wpa2:1;

    /** The signal strength of the beacon */
    signed char rssi;
} dna_wlan_scan_result_t;

/** Start Application Framework
 *
 * This is the main function that starts the application framework
 *
 * WLAN initialization
 * TCP/IP stack initialization
 * WLAN framework start
 *
 * Notice: This is blocking operation until WPA supplicant ready.
 *
 * \return DNA_SUCCESS on success
 * \return error code otherwise (dna_errno.h)
 */
int dna_wlan_framework_start(void);

/** Start Station Interface with Network information
 *
 * Connect the station interface to the network provided.
 *
 * \return DNA_SUCCESS on success
 * \return error code otherwise (dna_errno.h)
 *  It will return -DNA_FAIL when call again after success.
 *
 * \note This is a asynchronous interface, user need call dna_wlan_connect_status to check whether connect AP success.
 *  If you want to connect other network, however, current network was connecting, then *MUST* call dna_wlan_sta_stop first.
 */
int dna_wlan_sta_start(dna_wifi_network_t * network);

/** Stop Station Interface
 *
 * This stops the station interface that was started using the dna_wlan_sta_start()
 * function. After this, the application framework goes back
 * to the disconnected state.
 *
 * \return DNA_SUCCESS on success
 * \return error code otherwise (dna_errno.h)
 */
int dna_wlan_sta_stop(void);

/** Retrieve the status information of the station interface.
 *
 *  \return 1 if station interface is in DNA_WLAN_STA_CONNECTED state.
 *  \return 0 otherwise.
 */
int dna_wlan_connect_status(void);

/** Start micro-AP Network with DHCP Server
 *
 * This provides the facility to start the micro-AP network.
 *
 * This starts a default micro-AP network with WPA2 security and start a DHCP
 * Server on this interface. 
 *
 * If NULL is passed as the passphrase, an Open, unsecured micro-AP network is
 * started.
 * 
 * \param[in] ssid the micro-AP SSID
 * \param[in] wpa2_passphrase the WPA2 passphrase of the micro-AP.
 *				Pass NULL for an Open network.
 * \param[in] ipconfig the micro-AP ipconfig information (such as gateway ip address)
 * \param[in] channel is the channel number which WIFI driver is using for the specific wireless port.
 *                      The channel number range is 1~14 for 2.4G .The details channels number is determined 
 *                      by country region and BG Channel Table setting in the profile.
 *                  if channel = 0, then system auto select.
 *
 * \return DNA_SUCCESS on success
 * \return error code otherwise (dna_errno.h)
 *
 * \note This is a asynchronous interface, user need call dna_wlan_ap_status to check whether micro-AP started.
 */
int dna_wlan_ap_start(
    char * ssid, char * wpa2_pass,
    dna_ipconfig_t * ipconfig, int channel);

/** Stop micro-AP Network
 *
 * This stops the micro-AP network that was started using the dna_wlan_ap_start()
 * function. 
 *
 * \return DNA_SUCCESS on success
 * \return error code otherwise (dna_errno.h)
 *
 */
int dna_wlan_ap_stop(void);

/** Retrieve the status information of the micro-AP interface.
 *
 *  \return 1 if micro-AP interface is in DNA_WLAN_AP_STARTED state.
 *  \return 0 otherwise. 
 */
int dna_wlan_ap_status(void);

/** Scan for wireless networks.
 *
 *  When this function is called, the WLAN Connection Manager starts scan 
 *  for wireless networks. On completion of the scan the WLAN Connection Manager 
 *  will call the specified callback function \a cb. The callback function can then
 *  retrieve the scan results by using the \ref dna_wlan_get_scan_result() function.
 *
 *  \param[in] cb A pointer to the function that will be called to handle scan
 *         results when they are available.
 *
 *  \return DNA_SUCCESS if successful.
 *  \return -DNA_E_INVAL if \a cb is NULL,
 *  \return -DNA_FAIL if an internal error has occurred and
 *           the system is unable to scan.
 */
int dna_wlan_scan(int (* cb)(unsigned int count));

/** Scan for wireless networks with SSID.
 *
 *  When this function is called, the WLAN Connection Manager starts scan 
 *  for wireless networks. On completion of the scan the WLAN Connection Manager 
 *  will call the specified callback function \a cb. The callback function can then
 *  retrieve the scan results by using the \ref dna_wlan_get_scan_result() function.
 *
 *  \param[in] ssid Target AP SSID
 *  \param[in] ssid_len Target AP SSID length
 *  \param[in] cb A pointer to the function that will be called to handle scan
 *         results when they are available.
 *
 *  \return DNA_SUCCESS if successful.
 *  \return -DNA_E_INVAL if \a cb is NULL,
 *  \return -DNA_FAIL if an internal error has occurred and
 *           the system is unable to scan.
 */
int dna_wlan_scan_with_ssid(const char * ssid, int ssid_len, int (* cb)(unsigned int count));

/** Retrieve a scan result.
 *
 *  This function may be called to retrieve scan results when the WLAN
 *  Connection Manager has finished scanning. It must be called from within the
 *  scan result callback (see \ref dna_wlan_scan()) as scan results are valid 
 *  only in that context. The callback argument 'count' provides the number 
 *  of scan results that may be retrieved and \ref dna_wlan_get_scan_result() may 
 *  be used to retrieve scan results at \a index 0 through that number.
 *
 *  \note This function may only be called in the context of the scan results
 *  callback.
 *
 *  \note Calls to this function are synchronous.
 *
 *  \param[in] index The scan result to retrieve.
 *  \param[out] res A pointer to the \ref dna_wlan_scan_result_t where the scan
 *              result information will be copied.
 *
 *  \return DNA_SUCCESS if successful.
 *  \return -DNA_E_INVAL if \a res is NULL
 *  \return -DNA_FAIL if the scan result at \a
 *          index could not be retrieved (that is, \a index 
 *          is out of range).
 */
int dna_wlan_get_scan_result(
    unsigned int index,
    dna_wlan_scan_result_t * res);

/** Retrieve the wireless MAC address of station/micro-AP interface.
 *
 *  This function copies the MAC address of the wireless interface to
 *  the 6-byte array pointed to by \a mac.  In the event of an error, nothing 
 *  is copied to \a mac.
 *
 *  \param[out] mac A pointer to a 6-byte array where the MAC address will be
 *              copied.
 *
 *  \return DNA_SUCCESS if the MAC address was copied.
 *  \return -DNA_E_INVAL if \a mac is NULL.
 */
int dna_wlan_get_mac_address(unsigned char * mac);

/** Retrieve the IP address of the station interface.
 *
 *  This function retrieves the IP address
 *  of the station interface and copies it to the memory 
 *  location pointed to by \a ip.
 * 
 *  \note This function may only be called when the station interface is in the
 *  \ref DNA_WLAN_STA_CONNECTED state.
 *
 *  \param[out] ip A pointer to the ip address (4-byte, Network byte order).
 *
 *  \return DNA_SUCCESS if successful.
 *  \return -DNA_E_INVAL if \a ip is NULL.
 *  \return -DNA_FAIL if an internal error
 *          occurred when retrieving IP address information from the
 *          TCP/IP stack.
 */
int dna_wlan_get_ip_address(unsigned int * ip);

int dna_wlan_set_ip_address(unsigned int ip);


/** Retrieve the current network configuration of station or micro-AP interface.
 *
 *  This function retrieves the current network configuration of station
 *  interface when the station interface is in the \ref DNA_WLAN_STA_CONNECTED or DNA_WLAN_AP_STARTED
 *  state.
 *
 *  \param[out] network A pointer to the \ref dna_wifi_network_t. 
 *
 *  \return DNA_SUCCESS if successful.
 *  \return -DNA_E_INVAL if \a network is NULL.
 *  \return DNA_FAIL if the WLAN Connection Manager was
 *          not running or not in the \ref DNA_WLAN_STA_CONNECTED or DNA_WLAN_AP_STARTED state.
 */
int dna_wlan_get_current_network(dna_wifi_network_t * network);

/* Get current network rssi */
int dna_wlan_get_current_rssi(signed char * rssi);

/* WiFi Chip enter sniffer mode */
int dna_wlan_sniffer_enter(
        int filter,
        dna_wlan_sniffer_handler_t sniffer_callback);

int dna_wlan_extend_sniffer_enter(
        int filter,
        dna_wlan_extend_sniffer_handler_t sniffer_callback);


/* WiFi Chip exit sniffer mode */
int dna_wlan_sniffer_exit(void);

/* WiFi Chip sniffer start with specified channel */
int dna_wlan_sniffer_start(unsigned char channel);

/* WiFi Chip sniffer suspend:
*  Some hardware platform, it must stop or suspend current sniffer channel
*  Before switching channel.
*/
int dna_wlan_sniffer_stop(void);

/*
*  WiFi Chip STA or AP mode grab specified management frame Enable.
*  
*  @ vendor_ie: optional
*/
int dna_wlan_frame_filter_register(
        unsigned short frame_type,
        dna_wlan_vender_ie_t * vendor_ie,
        dna_wlan_sniffer_handler_t callback);

/*
*  WiFi Chip STA or AP mode grap specified management frame Disable.
*/
int dna_wlan_frame_filter_unregister(void);

/*
*   WiFi Chip send 802.11 raw packet (such as beacon/probe so on).
*   @ payload: 802.11 header + body
*   @ length: packet length
*
*   Return 0 means send packet success, otherwise fail.
*/
int dna_wlan_send_raw_packet(unsigned char * payload, unsigned int length);

/*
*   set wlan radio : on / off ;   DNA_WLAN_RADIO_OFF/DNA_WLAN_RADIO_ON
*   @ on_off: 0 --  DNA_WLAN_RADIO_OFF   shut down RF , 
*                 1 --  DNA_WLAN_RADIO_ON     open RF ;
*  
*   Return 0 means  success, otherwise fail.
*/
int dna_wlan_radio_set(unsigned char on_off);

/*
*  WiFi chip powerup (exit lower-power mode).
*/
int dna_wlan_powerup(void);

/*
*  WiFi chip powerdown (enter to lower-power mode).
*/
int dna_wlan_powerdown(void);

int dna_wlan_set_channel(unsigned char channel);

int dna_wlan_get_channel(unsigned char * channel);

int dna_dhcp_hostname_set(char * hostname);

int dna_dhcp_vendorname_set(char * vendorname);

/** Start WPS PBC network configuration.
 *
 * \network: output param, wps config result
 * \return DNA_SUCCESS on success
 * \return error code otherwise (dna_errno.h)
 */
int dna_wlan_wps_start(dna_wifi_network_t * network);

#ifdef __cplusplus
}
#endif

#endif
