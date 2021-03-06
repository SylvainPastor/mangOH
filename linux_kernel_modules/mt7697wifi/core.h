/*
 * Copyright (c) 2017 Sierra Wireless Corporation
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _MT7697_CORE_H_
#define _MT7697_CORE_H_

#include <net/cfg80211.h>

#include "wifi_api.h"
#include "wmi.h"

#define DRVNAME				"mt7697core"

#define MT7697_MAC80211_QUEUE_TX	0
#define MT7697_MAC80211_QUEUE_RX	1

#define MT7697_CH_MAX_2G_CHANNEL	14	/* Max channel in 2G band */
#define MT7697_CH_MIN_5G_CHANNEL	34

#define MT7697_SCAN_MAX_ITEMS		16
#define MT7697_IFACE_MAX_CNT		2
#define MT7697_MAX_MC_FILTERS_PER_LIST 	7
#define MT7697_MAX_COOKIE_NUM		180
#define MT7697_TX_TIMEOUT      		10
#define MT7697_DISCON_TIMER_INTVAL      10000  /* in msec */

#define MT7697_KEY_SEQ_LEN 		8
#define MT7697_MAX_KEY_INDEX   		3

enum mt7697_dev_state {
	WMI_ENABLED,
	WMI_READY,
	WMI_CTRL_EP_FULL,
	TESTMODE,
	DESTROY_IN_PROGRESS,
	SKIP_SCAN,
	ROAM_TBL_PEND,
	FIRST_BOOT,
	RECOVERY_CLEANUP,
};

enum mt7697_sme_state {
	SME_DISCONNECTED,
	SME_CONNECTING,
	SME_CONNECTED
};

enum mt7697_vif_state {
        CONNECTED,
        CONNECT_PEND,
        WMM_ENABLED,
        NETQ_STOPPED,
        DTIM_EXPIRED,
        CLEAR_BSSFILTER_ON_BEACON,
        DTIM_PERIOD_AVAIL,
        WLAN_ENABLED,
        STATS_UPDATE_PEND,
        HOST_SLEEP_MODE_CMD_PROCESSED,
        NETDEV_MCAST_ALL_ON,
        NETDEV_MCAST_ALL_OFF,
        SCHED_SCANNING,
};

struct mt7697_cookie {
	struct sk_buff *skb;
	struct mt7697_cookie *arc_list_next;
};

struct mt7697_cfg80211_info {
	struct device *dev;
        struct wiphy *wiphy;
	struct semaphore sem;
        
	struct platform_device *hif_priv;
	const struct mt7697q_if_ops *hif_ops;

	void* txq_hdl;
	void* rxq_hdl;

	struct work_struct init_work;

	struct mt7697_cookie *cookie_list;
	u32 cookie_count;
	struct mt7697_cookie cookie_mem[MT7697_MAX_COOKIE_NUM];

	struct work_struct tx_work;
	u8 tx_data[IEEE80211_MAX_DATA_LEN];
	u8 rx_data[IEEE80211_MAX_DATA_LEN];
	u8 probe_data[IEEE80211_MAX_DATA_LEN];

	struct mt7697_rsp_hdr rsp;
	
	enum mt7697_radio_state radio_state;
	enum mt7697_wifi_phy_mode_t wireless_mode;
	struct mac_address mac_addr;
	struct mt7697_wifi_config_t wifi_config;
	int listen_interval;
	enum mt7697_wifi_rx_filter_t rx_filter;
	u8 pmkid[MT7697_WIFI_LENGTH_PMK];

	struct list_head vif_list;
	spinlock_t vif_list_lock;
	u8 num_vif;
	unsigned int vif_max;
	u8 max_norm_iface;
	u8 avail_idx_map;

	bool wiphy_registered;
	bool ibss_if_active;
	u32 connect_ctrl_flags;
	unsigned long flag;
};

struct mt7697_key {
	u8 key[WLAN_MAX_KEY_LEN];
	u8 key_len;
	u8 seq[MT7697_KEY_SEQ_LEN];
	u8 seq_len;
	u32 cipher;
};

struct mt7697_vif {
	struct list_head list;
	struct wireless_dev wdev;
	struct net_device *ndev;
	struct mt7697_cfg80211_info *cfg;

	/* Lock to protect vif specific net_stats and flags */
	spinlock_t if_lock;
	u8 fw_vif_idx;
	unsigned long flags;

	int ssid_len;
	u8 ssid[IEEE80211_MAX_SSID_LEN];
	u8 bssid[ETH_ALEN];
	u8 req_bssid[ETH_ALEN];
	u16 ch_hint;
 
	struct work_struct disconnect_work;
	struct timer_list disconnect_timer;

	enum mt7697_wifi_auth_mode_t auth_mode;
	u8 auto_connect;
	enum mt7697_wifi_encrypt_type_t prwise_crypto;
	u8 prwise_crypto_len;
	enum mt7697_wifi_encrypt_type_t grp_crypto;
	u8 grp_crypto_len;

	u8 def_txkey_index;
	struct mt7697_key keys[MT7697_MAX_KEY_INDEX + 1];

	struct cfg80211_scan_request *scan_req;
	bool probe_req_report;
	enum mt7697_sme_state sme_state;
	int reconnect_flag;
	u8 listen_intvl_t;
};

static inline struct wiphy *cfg_to_wiphy(struct mt7697_cfg80211_info *cfg)
{
        return cfg->wiphy;
}

static inline struct mt7697_cfg80211_info *wiphy_to_cfg(struct wiphy *w)
{
        return (struct mt7697_cfg80211_info *)(wiphy_priv(w));
}

static inline struct mt7697_cfg80211_info *mt7697_priv(struct net_device *ndev)
{
	return ((struct mt7697_vif*)netdev_priv(ndev))->cfg;
}

static inline struct mt7697_vif *mt7697_vif_from_wdev(struct wireless_dev *wdev)
{
        return container_of(wdev, struct mt7697_vif, wdev);
}

void mt7697_init_netdev(struct net_device*);

struct wireless_dev *mt7697_interface_add(struct mt7697_cfg80211_info*, 
	const char*, enum nl80211_iftype, u8 fw_vif_idx);
void mt7697_tx_work(struct work_struct *);
int mt7697_data_tx(struct sk_buff*, struct net_device*);

void mt7697_disconnect_timer_hndlr(unsigned long);
int mt7697_disconnect(struct mt7697_vif*);

struct mt7697_cookie *mt7697_alloc_cookie(struct mt7697_cfg80211_info*);
void mt7697_free_cookie(struct mt7697_cfg80211_info*, struct mt7697_cookie*);

#endif
