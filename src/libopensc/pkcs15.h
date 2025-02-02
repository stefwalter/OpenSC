/*
 * pkcs15.h: OpenSC PKCS#15 header file
 *
 * Copyright (C) 2001, 2002  Juha Yrjölä <juha.yrjola@iki.fi>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _OPENSC_PKCS15_H
#define _OPENSC_PKCS15_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libopensc/opensc.h"

#define SC_PKCS15_CACHE_DIR		".eid"

#define SC_PKCS15_PIN_MAGIC		0x31415926
#define SC_PKCS15_MAX_PINS		8
#define SC_PKCS15_MAX_LABEL_SIZE	255
#define SC_PKCS15_MAX_ID_SIZE		255

/* When changing this value, change also initialisation of the 
 * static ASN1 variables, that use this macro,
 * like for example, 'c_asn1_access_control_rules' 
 * in src/libopensc/asn1.c */
#define SC_PKCS15_MAX_ACCESS_RULES      8

struct sc_pkcs15_id {
	u8 value[SC_PKCS15_MAX_ID_SIZE];
	size_t len;
};
typedef struct sc_pkcs15_id sc_pkcs15_id_t;

#define SC_PKCS15_CO_FLAG_PRIVATE	0x00000001
#define SC_PKCS15_CO_FLAG_MODIFIABLE	0x00000002
#define SC_PKCS15_CO_FLAG_OBJECT_SEEN	0x80000000 /* for PKCS #11 module */

#define SC_PKCS15_PIN_FLAG_CASE_SENSITIVE		0x0001
#define SC_PKCS15_PIN_FLAG_LOCAL			0x0002
#define SC_PKCS15_PIN_FLAG_CHANGE_DISABLED		0x0004
#define SC_PKCS15_PIN_FLAG_UNBLOCK_DISABLED		0x0008
#define SC_PKCS15_PIN_FLAG_INITIALIZED			0x0010
#define SC_PKCS15_PIN_FLAG_NEEDS_PADDING		0x0020
#define SC_PKCS15_PIN_FLAG_UNBLOCKING_PIN		0x0040
#define SC_PKCS15_PIN_FLAG_SO_PIN			0x0080
#define SC_PKCS15_PIN_FLAG_DISABLE_ALLOW		0x0100
#define SC_PKCS15_PIN_FLAG_INTEGRITY_PROTECTED		0x0200
#define SC_PKCS15_PIN_FLAG_CONFIDENTIALITY_PROTECTED	0x0400
#define SC_PKCS15_PIN_FLAG_EXCHANGE_REF_DATA		0x0800

#define SC_PKCS15_PIN_TYPE_BCD				0
#define SC_PKCS15_PIN_TYPE_ASCII_NUMERIC		1
#define SC_PKCS15_PIN_TYPE_UTF8				2
#define SC_PKCS15_PIN_TYPE_HALFNIBBLE_BCD		3
#define SC_PKCS15_PIN_TYPE_ISO9564_1			4

#define SC_PKCS15_PIN_AUTH_TYPE_PIN			0
#define SC_PKCS15_PIN_AUTH_TYPE_BIOMETRIC		1
#define SC_PKCS15_PIN_AUTH_TYPE_AUTH_KEY		2
#define SC_PKCS15_PIN_AUTH_TYPE_SM_KEY			3

/* PinAttributes as they defined in PKCS#15 v1.1 for PIN authentication object */
struct sc_pkcs15_pin_attributes {
	unsigned int  flags, type;
	size_t  min_length, stored_length, max_length;
	int  reference;
	u8  pad_char;
};
/* AuthKeyAttributes of the authKey authentication object */
struct sc_pkcs15_authkey_attributes {
	int derived;
	struct sc_pkcs15_id skey_id;
};
/* BiometricAttributes of the biometricTemplate authentication object */
struct sc_pkcs15_biometric_attributes {
	unsigned int flags;
	struct sc_object_id template_id;
	/* ... */
};
struct sc_pkcs15_auth_info {
	/* CommonAuthenticationObjectAttributes */
	struct sc_pkcs15_id  auth_id;

	/* AuthObjectAttributes */
	struct sc_path  path;
	unsigned auth_type;
	union {
		struct sc_pkcs15_pin_attributes pin;
		struct sc_pkcs15_biometric_attributes bio;
		struct sc_pkcs15_authkey_attributes authkey;
	} attrs;

	/* authentication method: CHV, SEN, SYMBOLIC, ... */
	unsigned int  auth_method;

	int  tries_left, max_tries;
 };
typedef struct sc_pkcs15_auth_info sc_pkcs15_auth_info_t;

#define SC_PKCS15_ALGO_OP_COMPUTE_CHECKSUM	0x01
#define SC_PKCS15_ALGO_OP_COMPUTE_SIGNATURE	0x02
#define SC_PKCS15_ALGO_OP_VERIFY_CHECKSUM	0x04
#define SC_PKCS15_ALGO_OP_VERIFY_SIGNATURE	0x08
#define SC_PKCS15_ALGO_OP_ENCIPHER		0x10
#define SC_PKCS15_ALGO_OP_DECIPHER		0x20
#define SC_PKCS15_ALGO_OP_HASH			0x40
#define SC_PKCS15_ALGO_OP_GENERATE_KEY		0x80

/* A large integer, big endian notation */
struct sc_pkcs15_bignum {
	u8 *		data;
	size_t		len;
};
typedef struct sc_pkcs15_bignum sc_pkcs15_bignum_t;

struct sc_pkcs15_der {
	u8 *		value;
	size_t		len;
};
typedef struct sc_pkcs15_der sc_pkcs15_der_t;

struct sc_pkcs15_pubkey_rsa {
	sc_pkcs15_bignum_t modulus;
	sc_pkcs15_bignum_t exponent;
};

struct sc_pkcs15_prkey_rsa {
	/* public components */
	sc_pkcs15_bignum_t modulus;
	sc_pkcs15_bignum_t exponent;

	/* private components */
	sc_pkcs15_bignum_t d;
	sc_pkcs15_bignum_t p;
	sc_pkcs15_bignum_t q;

	/* optional CRT elements */
	sc_pkcs15_bignum_t iqmp;
	sc_pkcs15_bignum_t dmp1;
	sc_pkcs15_bignum_t dmq1;
};

struct sc_pkcs15_pubkey_dsa {
	sc_pkcs15_bignum_t pub;
	sc_pkcs15_bignum_t p;
	sc_pkcs15_bignum_t q;
	sc_pkcs15_bignum_t g;
};

struct sc_pkcs15_prkey_dsa {
	/* public components */
	sc_pkcs15_bignum_t pub;
	sc_pkcs15_bignum_t p;
	sc_pkcs15_bignum_t q;
	sc_pkcs15_bignum_t g;

	/* private key */
	sc_pkcs15_bignum_t priv;
};

/* 
 * The ecParameters can be presented as
 * - named curve;
 * - OID of named curve;
 * - implicit parameters.
 */
struct sc_pkcs15_ec_parameters {
	char *named_curve;
	struct sc_object_id id;
	sc_pkcs15_der_t der;
	size_t field_length; /* in bits */
};

struct sc_pkcs15_gost_parameters {
	struct sc_object_id key;
	struct sc_object_id hash;
	struct sc_object_id cipher;
};

struct sc_pkcs15_pubkey_ec {
	struct sc_pkcs15_ec_parameters params;
	sc_pkcs15_der_t		ecpointQ; /* note this is der */
};

struct sc_pkcs15_prkey_ec {
	struct sc_pkcs15_ec_parameters params;
	sc_pkcs15_bignum_t	privateD; /* note this is bignum */
};

struct sc_pkcs15_pubkey_gostr3410 {
	struct sc_pkcs15_gost_parameters params;
	sc_pkcs15_bignum_t xy;
};

struct sc_pkcs15_prkey_gostr3410 {
	struct sc_pkcs15_gost_parameters params;
	sc_pkcs15_bignum_t d;
};

struct sc_pkcs15_pubkey {
	int algorithm;
	struct sc_algorithm_id * alg_id;

	/* Decoded key */
	union {
		struct sc_pkcs15_pubkey_rsa rsa;
		struct sc_pkcs15_pubkey_dsa dsa;
		struct sc_pkcs15_pubkey_ec ec;
		struct sc_pkcs15_pubkey_gostr3410 gostr3410;
	} u;

	/* DER encoded raw key */
	sc_pkcs15_der_t data;
};
typedef struct sc_pkcs15_pubkey sc_pkcs15_pubkey_t;

struct sc_pkcs15_prkey {
	unsigned int algorithm;
/* TODO do we need:	struct sc_algorithm_id * alg_id; */

	union {
		struct sc_pkcs15_prkey_rsa rsa;
		struct sc_pkcs15_prkey_dsa dsa;
		struct sc_pkcs15_prkey_ec ec;
		struct sc_pkcs15_prkey_gostr3410 gostr3410;
	} u;
};
typedef struct sc_pkcs15_prkey sc_pkcs15_prkey_t;

/* Enveloped objects can be used to provide additional
 * protection to non-native private keys */
struct sc_pkcs15_enveloped_data {
	/* recipient info */
	sc_pkcs15_id_t id;		/* key ID */
	struct sc_algorithm_id ke_alg;	/* key-encryption algo */
	u8 *key;			/* encrypted key */
	size_t key_len;
	struct sc_algorithm_id ce_alg;	/* content-encryption algo */
	u8 *content;			/* encrypted content */
	size_t content_len;
};

struct sc_pkcs15_cert {
	int version;
	u8 *serial;
	size_t serial_len;
	u8 *issuer;
	size_t issuer_len;
	u8 *subject;
	size_t subject_len;
	u8 *crl;
	size_t crl_len;

	struct sc_pkcs15_pubkey * key;
	u8 *data;	/* DER encoded raw cert */
	size_t data_len;
};
typedef struct sc_pkcs15_cert sc_pkcs15_cert_t;

struct sc_pkcs15_cert_info {
	struct sc_pkcs15_id id;	/* correlates to private key id */
	int authority;		/* boolean */
	/* identifiers [2] SEQUENCE OF CredentialIdentifier{{KeyIdentifiers}} */
	struct sc_path path;

	sc_pkcs15_der_t value;
};
typedef struct sc_pkcs15_cert_info sc_pkcs15_cert_info_t;

struct sc_pkcs15_data {
	u8 *data;	/* DER encoded raw data object */
	size_t data_len;
};
typedef struct sc_pkcs15_data sc_pkcs15_data_t;

struct sc_pkcs15_data_info {
	/* FIXME: there is no pkcs15 ID in DataType */
	struct sc_pkcs15_id id;

	/* Identify the application:
	 * either or both may be set */
	char app_label[SC_PKCS15_MAX_LABEL_SIZE];
	struct sc_object_id app_oid;

	struct sc_path path;
};
typedef struct sc_pkcs15_data_info sc_pkcs15_data_info_t;

#define SC_PKCS15_PRKEY_USAGE_ENCRYPT		0x01
#define SC_PKCS15_PRKEY_USAGE_DECRYPT		0x02
#define SC_PKCS15_PRKEY_USAGE_SIGN		0x04
#define SC_PKCS15_PRKEY_USAGE_SIGNRECOVER	0x08
#define SC_PKCS15_PRKEY_USAGE_WRAP		0x10
#define SC_PKCS15_PRKEY_USAGE_UNWRAP		0x20
#define SC_PKCS15_PRKEY_USAGE_VERIFY		0x40
#define SC_PKCS15_PRKEY_USAGE_VERIFYRECOVER	0x80
#define SC_PKCS15_PRKEY_USAGE_DERIVE		0x100
#define SC_PKCS15_PRKEY_USAGE_NONREPUDIATION	0x200

#define SC_PKCS15_PRKEY_ACCESS_SENSITIVE	0x01
#define SC_PKCS15_PRKEY_ACCESS_EXTRACTABLE	0x02
#define SC_PKCS15_PRKEY_ACCESS_ALWAYSSENSITIVE	0x04
#define SC_PKCS15_PRKEY_ACCESS_NEVEREXTRACTABLE	0x08
#define SC_PKCS15_PRKEY_ACCESS_LOCAL		0x10

#define SC_PKCS15_PARAMSET_GOSTR3410_A          1
#define SC_PKCS15_PARAMSET_GOSTR3410_B          2
#define SC_PKCS15_PARAMSET_GOSTR3410_C          3

#define SC_PKCS15_GOSTR3410_KEYSIZE             256

struct sc_pkcs15_keyinfo_gostparams
{
	unsigned int gostr3410, gostr3411, gost28147;
};

/* AccessMode bit definitions specified in PKCS#15 v1.1
 * and extended by IAS/ECC v1.0.1 specification. */
#define SC_PKCS15_ACCESS_RULE_MODE_READ         0x01
#define SC_PKCS15_ACCESS_RULE_MODE_UPDATE       0x02
#define SC_PKCS15_ACCESS_RULE_MODE_EXECUTE      0x04
#define SC_PKCS15_ACCESS_RULE_MODE_DELETE       0x08
#define SC_PKCS15_ACCESS_RULE_MODE_ATTRIBUTE    0x10
#define SC_PKCS15_ACCESS_RULE_MODE_PSO_CDS      0x20
#define SC_PKCS15_ACCESS_RULE_MODE_PSO_VERIFY   0x40
#define SC_PKCS15_ACCESS_RULE_MODE_PSO_DECRYPT  0x80
#define SC_PKCS15_ACCESS_RULE_MODE_PSO_ENCRYPT  0x100
#define SC_PKCS15_ACCESS_RULE_MODE_INT_AUTH     0x200
#define SC_PKCS15_ACCESS_RULE_MODE_EXT_AUTH     0x400

struct sc_pkcs15_accessrule {
	unsigned access_mode;
	struct sc_pkcs15_id auth_id;
};
typedef struct sc_pkcs15_accessrule sc_pkcs15_accessrule_t;


struct sc_pkcs15_key_params {
	void   *data;
	size_t len;
	void (*free_params)(void *);
};

struct sc_pkcs15_prkey_info {
	struct sc_pkcs15_id id;	/* correlates to public certificate id */
	unsigned int usage, access_flags;
	int native, key_reference;
	/* convert to union if other types are supported */
	size_t modulus_length; /* RSA */
	size_t field_length;   /* EC in bits */

	unsigned int algo_refs[SC_MAX_SUPPORTED_ALGORITHMS];

	struct sc_pkcs15_der subject;

	struct sc_pkcs15_key_params params;

	struct sc_path path;
};
typedef struct sc_pkcs15_prkey_info sc_pkcs15_prkey_info_t;

struct sc_pkcs15_pubkey_info {
	struct sc_pkcs15_id id;	/* correlates to private key id */
	unsigned int usage, access_flags;
	int native, key_reference;
	/* convert to union if other types are supported */
	size_t modulus_length; /* RSA */
	size_t field_length;   /* EC in bits */

	unsigned int algo_refs[SC_MAX_SUPPORTED_ALGORITHMS];

	struct sc_pkcs15_der subject;

	struct sc_pkcs15_key_params params;

	struct sc_path path;
};
typedef struct sc_pkcs15_pubkey_info sc_pkcs15_pubkey_info_t;

#define SC_PKCS15_TYPE_CLASS_MASK		0xF00

#define SC_PKCS15_TYPE_PRKEY			0x100
#define SC_PKCS15_TYPE_PRKEY_RSA		0x101
#define SC_PKCS15_TYPE_PRKEY_DSA		0x102
#define SC_PKCS15_TYPE_PRKEY_GOSTR3410		0x103
#define SC_PKCS15_TYPE_PRKEY_EC		0x104

#define SC_PKCS15_TYPE_PUBKEY			0x200
#define SC_PKCS15_TYPE_PUBKEY_RSA		0x201
#define SC_PKCS15_TYPE_PUBKEY_DSA		0x202
#define SC_PKCS15_TYPE_PUBKEY_GOSTR3410		0x203
#define SC_PKCS15_TYPE_PUBKEY_EC		0x204

#define SC_PKCS15_TYPE_CERT			0x400
#define SC_PKCS15_TYPE_CERT_X509		0x401
#define SC_PKCS15_TYPE_CERT_SPKI		0x402

#define SC_PKCS15_TYPE_DATA_OBJECT		0x500
#define SC_PKCS15_TYPE_AUTH			0x600
#define SC_PKCS15_TYPE_AUTH_PIN			0x601

#define SC_PKCS15_TYPE_TO_CLASS(t)		(1 << ((t) >> 8))
#define SC_PKCS15_SEARCH_CLASS_PRKEY		0x0002U
#define SC_PKCS15_SEARCH_CLASS_PUBKEY		0x0004U
#define SC_PKCS15_SEARCH_CLASS_CERT		0x0010U
#define SC_PKCS15_SEARCH_CLASS_DATA		0x0020U
#define SC_PKCS15_SEARCH_CLASS_AUTH		0x0040U

struct sc_pkcs15_object {
	unsigned int type;
	/* CommonObjectAttributes */
	char label[SC_PKCS15_MAX_LABEL_SIZE];	/* zero terminated */
	unsigned int flags;
	struct sc_pkcs15_id auth_id;

	int usage_counter;
	int user_consent;

	struct sc_pkcs15_accessrule access_rules[SC_PKCS15_MAX_ACCESS_RULES];

	/* Object type specific data */
	void *data;
	/* emulated object pointer */
	void *emulated;


	struct sc_pkcs15_df *df; /* can be NULL, if object is 'floating' */
	struct sc_pkcs15_object *next, *prev; /* used only internally */
	
	struct sc_pkcs15_der content;
};
typedef struct sc_pkcs15_object sc_pkcs15_object_t;

/* PKCS #15 DF types */
#define SC_PKCS15_PRKDF			0
#define SC_PKCS15_PUKDF			1
#define SC_PKCS15_PUKDF_TRUSTED		2
#define SC_PKCS15_SKDF			3
#define SC_PKCS15_CDF			4
#define SC_PKCS15_CDF_TRUSTED		5
#define SC_PKCS15_CDF_USEFUL		6
#define SC_PKCS15_DODF			7
#define SC_PKCS15_AODF			8
#define SC_PKCS15_DF_TYPE_COUNT		9

struct sc_pkcs15_card;

struct sc_pkcs15_df {
	struct sc_path path;
	int record_length;
	unsigned int type;
	int enumerated;

	struct sc_pkcs15_df *next, *prev;
};
typedef struct sc_pkcs15_df sc_pkcs15_df_t;

struct sc_pkcs15_unusedspace {
	sc_path_t path;
	sc_pkcs15_id_t auth_id;

	struct sc_pkcs15_unusedspace *next, *prev;
};
typedef struct sc_pkcs15_unusedspace sc_pkcs15_unusedspace_t;

#define SC_PKCS15_CARD_MAGIC		0x10203040

typedef struct sc_pkcs15_sec_env_info {
	int			se;
	struct sc_object_id	owner;
	struct sc_aid aid;
} sc_pkcs15_sec_env_info_t;

typedef struct sc_pkcs15_tokeninfo {
	unsigned int version;
	unsigned int flags;
	char *label;
	char *serial_number;
	char *manufacturer_id;	
	char *last_update;
	char *preferred_language;
	sc_pkcs15_sec_env_info_t **seInfo;
	size_t num_seInfo;

	struct sc_supported_algo_info supported_algos[SC_MAX_SUPPORTED_ALGORITHMS];
} sc_pkcs15_tokeninfo_t;

struct sc_pkcs15_operations   {
	int (*parse_df)(struct sc_pkcs15_card *, struct sc_pkcs15_df *);
	void (*clear)(struct sc_pkcs15_card *);
	int (*get_guid)(struct sc_pkcs15_card *, const struct sc_pkcs15_object *, 
			char *, size_t);
};

typedef struct sc_pkcs15_card {
	sc_card_t *card;
	unsigned int flags;

	struct sc_app_info *app;

	sc_file_t *file_app;
	sc_file_t *file_tokeninfo, *file_odf, *file_unusedspace;

	struct sc_pkcs15_df *df_list;
	struct sc_pkcs15_object *obj_list;
	sc_pkcs15_tokeninfo_t *tokeninfo;
	sc_pkcs15_unusedspace_t *unusedspace_list;
	int unusedspace_read;

	struct sc_pkcs15_card_opts {
		int use_file_cache;
		int use_pin_cache;
		int pin_cache_counter;
	} opts;


	unsigned int magic;

	void *dll_handle;		/* shared lib for emulated cards */

	struct sc_pkcs15_operations ops;

} sc_pkcs15_card_t;

/* flags suitable for sc_pkcs15_tokeninfo_t */
#define SC_PKCS15_TOKEN_READONLY			0x01
#define SC_PKCS15_TOKEN_LOGIN_REQUIRED			0x02 /* Don't use */
#define SC_PKCS15_TOKEN_PRN_GENERATION			0x04
#define SC_PKCS15_TOKEN_EID_COMPLIANT			0x08

/* flags suitable for sc_pkcs15_card_t */
#define SC_PKCS15_CARD_FLAG_EMULATED			0x02000000

/* sc_pkcs15_bind:  Binds a card object to a PKCS #15 card object
 * and initializes a new PKCS #15 card object.  Will return
 * SC_ERROR_PKCS15_APP_NOT_FOUND, if the card hasn't got a
 * valid PKCS #15 file structure. */
int sc_pkcs15_bind(struct sc_card *card, struct sc_aid *aid,
		   struct sc_pkcs15_card **pkcs15_card);
/* sc_pkcs15_unbind:  Releases a PKCS #15 card object, and frees any
 * memory allocations done on the card object. */
int sc_pkcs15_unbind(struct sc_pkcs15_card *card);

int sc_pkcs15_get_objects(struct sc_pkcs15_card *card, unsigned int type,
			  struct sc_pkcs15_object **ret, size_t ret_count);
int sc_pkcs15_get_objects_cond(struct sc_pkcs15_card *card, unsigned int type,
			       int (* func)(struct sc_pkcs15_object *, void *),
			       void *func_arg,
			       struct sc_pkcs15_object **ret, size_t ret_count);
int sc_pkcs15_find_object_by_id(sc_pkcs15_card_t *, unsigned int,
				const sc_pkcs15_id_t *,
				sc_pkcs15_object_t **);

struct sc_pkcs15_card * sc_pkcs15_card_new(void);
void sc_pkcs15_card_free(struct sc_pkcs15_card *p15card);
void sc_pkcs15_card_clear(sc_pkcs15_card_t *p15card);

int sc_pkcs15_decipher(struct sc_pkcs15_card *p15card,
		       const struct sc_pkcs15_object *prkey_obj,
		       unsigned long flags,
		       const u8 *in, size_t inlen, u8 *out, size_t outlen);

int sc_pkcs15_compute_signature(struct sc_pkcs15_card *p15card,
				const struct sc_pkcs15_object *prkey_obj,
				unsigned long alg_flags, const u8 *in,
				size_t inlen, u8 *out, size_t outlen);

int sc_pkcs15_read_pubkey(struct sc_pkcs15_card *,
			const struct sc_pkcs15_object *,
			struct sc_pkcs15_pubkey **);
int sc_pkcs15_decode_pubkey_rsa(struct sc_context *,
	       		struct sc_pkcs15_pubkey_rsa *,
			const u8 *, size_t);
int sc_pkcs15_encode_pubkey_rsa(struct sc_context *,
			struct sc_pkcs15_pubkey_rsa *, u8 **, size_t *);
int sc_pkcs15_decode_pubkey_dsa(struct sc_context *,
	       		struct sc_pkcs15_pubkey_dsa *,
			const u8 *, size_t);
int sc_pkcs15_encode_pubkey_dsa(struct sc_context *,
			struct sc_pkcs15_pubkey_dsa *, u8 **, size_t *);
int sc_pkcs15_decode_pubkey_gostr3410(sc_context_t *,
		struct sc_pkcs15_pubkey_gostr3410 *, const u8 *, size_t);
int sc_pkcs15_encode_pubkey_gostr3410(sc_context_t *,
		struct sc_pkcs15_pubkey_gostr3410 *, u8 **, size_t *);
int sc_pkcs15_decode_pubkey_ec(struct sc_context *,
			struct sc_pkcs15_pubkey_ec *, const u8 *, size_t);
int sc_pkcs15_encode_pubkey_ec(struct sc_context *,
				struct sc_pkcs15_pubkey_ec *, u8 **, size_t *);
int sc_pkcs15_decode_pubkey(struct sc_context *,
	       		struct sc_pkcs15_pubkey *, const u8 *, size_t);
int sc_pkcs15_encode_pubkey(struct sc_context *,
			struct sc_pkcs15_pubkey *, u8 **, size_t *);
void sc_pkcs15_erase_pubkey(struct sc_pkcs15_pubkey *);
void sc_pkcs15_free_pubkey(struct sc_pkcs15_pubkey *);
int sc_pkcs15_pubkey_from_prvkey(struct sc_context *, struct sc_pkcs15_prkey *, 
			struct sc_pkcs15_pubkey **);
int sc_pkcs15_pubkey_from_cert(struct sc_context *, struct sc_pkcs15_der *, 
			struct sc_pkcs15_pubkey **);
int sc_pkcs15_pubkey_from_spki_filename(struct sc_context *, 
			char *, sc_pkcs15_pubkey_t ** );
int sc_pkcs15_pubkey_from_spki(struct sc_context *,
			sc_pkcs15_pubkey_t **, u8 *, size_t, int);
int sc_pkcs15_encode_prkey(struct sc_context *,
			struct sc_pkcs15_prkey *,
			u8 **, size_t *);
void sc_pkcs15_free_prkey(struct sc_pkcs15_prkey *prkey);
void sc_pkcs15_free_key_params(struct sc_pkcs15_key_params *params);

int sc_pkcs15_read_data_object(struct sc_pkcs15_card *p15card,
			       const struct sc_pkcs15_data_info *info,
			       struct sc_pkcs15_data **data_object_out);
int sc_pkcs15_find_data_object_by_id(struct sc_pkcs15_card *p15card,
				     const struct sc_pkcs15_id *id,
				     struct sc_pkcs15_object **out);
int sc_pkcs15_find_data_object_by_app_oid(struct sc_pkcs15_card *p15card,
					  const struct sc_object_id *app_oid,
					  struct sc_pkcs15_object **out);
int sc_pkcs15_find_data_object_by_name(struct sc_pkcs15_card *p15card,
				const char *app_label,
				const char *label,
				struct sc_pkcs15_object **out);
void sc_pkcs15_free_data_object(struct sc_pkcs15_data *data_object);

int sc_pkcs15_read_certificate(struct sc_pkcs15_card *card,
			       const struct sc_pkcs15_cert_info *info,
			       struct sc_pkcs15_cert **cert);
void sc_pkcs15_free_certificate(struct sc_pkcs15_cert *cert);
int sc_pkcs15_find_cert_by_id(struct sc_pkcs15_card *card,
			      const struct sc_pkcs15_id *id,
			      struct sc_pkcs15_object **out);
/* sc_pkcs15_create_cdf:  Creates a new certificate DF on a card pointed
 * by <card>.  Information about the file, such as the file ID, is read
 * from <file>.  <certs> has to be NULL-terminated. */
int sc_pkcs15_create_cdf(struct sc_pkcs15_card *card,
			 struct sc_file *file,
			 const struct sc_pkcs15_cert_info **certs);
int sc_pkcs15_create(struct sc_pkcs15_card *p15card, struct sc_card *card);

int sc_pkcs15_find_prkey_by_id(struct sc_pkcs15_card *card,
			       const struct sc_pkcs15_id *id,
			       struct sc_pkcs15_object **out);
int sc_pkcs15_find_prkey_by_id_usage(struct sc_pkcs15_card *card,
			       const struct sc_pkcs15_id *id,
			       unsigned int usage,
			       struct sc_pkcs15_object **out);
int sc_pkcs15_find_prkey_by_reference(sc_pkcs15_card_t *,
			       const sc_path_t *, int,
			       sc_pkcs15_object_t **);
int sc_pkcs15_find_pubkey_by_id(struct sc_pkcs15_card *card,
			       const struct sc_pkcs15_id *id,
			       struct sc_pkcs15_object **out);

int sc_pkcs15_verify_pin(struct sc_pkcs15_card *card,
			 struct sc_pkcs15_object *pin_obj,
			 const u8 *pincode, size_t pinlen);
int sc_pkcs15_change_pin(struct sc_pkcs15_card *card,
			 struct sc_pkcs15_object *pin_obj,
			 const u8 *oldpincode, size_t oldpinlen,
			 const u8 *newpincode, size_t newpinlen);
int sc_pkcs15_unblock_pin(struct sc_pkcs15_card *card,
			 struct sc_pkcs15_object *pin_obj,
			 const u8 *puk, size_t puklen,
			 const u8 *newpin, size_t newpinlen);
int sc_pkcs15_find_pin_by_auth_id(struct sc_pkcs15_card *card,
				  const struct sc_pkcs15_id *id,
				  struct sc_pkcs15_object **out);
int sc_pkcs15_find_pin_by_reference(struct sc_pkcs15_card *card,
				    const sc_path_t *path, int reference,
				    struct sc_pkcs15_object **out);
int sc_pkcs15_find_pin_by_type_and_reference(struct sc_pkcs15_card *card,
				    const sc_path_t *path, unsigned auth_method, 
				    int reference,
				    struct sc_pkcs15_object **out);
int sc_pkcs15_find_so_pin(struct sc_pkcs15_card *card,
			struct sc_pkcs15_object **out);

void sc_pkcs15_pincache_add(struct sc_pkcs15_card *, struct sc_pkcs15_object *, 
			const u8 *, size_t);
int sc_pkcs15_pincache_revalidate(struct sc_pkcs15_card *p15card, 
			const sc_pkcs15_object_t *obj);
void sc_pkcs15_pincache_clear(struct sc_pkcs15_card *p15card);

int sc_pkcs15_encode_dir(struct sc_context *ctx,
			struct sc_pkcs15_card *card,
			u8 **buf, size_t *buflen);
int sc_pkcs15_parse_tokeninfo(sc_context_t *ctx,
			sc_pkcs15_tokeninfo_t *ti,
			const u8 *buf, size_t blen);
int sc_pkcs15_encode_tokeninfo(struct sc_context *ctx,
			sc_pkcs15_tokeninfo_t *ti,
			u8 **buf, size_t *buflen);
int sc_pkcs15_encode_odf(struct sc_context *ctx,
			struct sc_pkcs15_card *card,
			u8 **buf, size_t *buflen);
int sc_pkcs15_encode_df(struct sc_context *ctx,
			struct sc_pkcs15_card *p15card,
			struct sc_pkcs15_df *df,
			u8 **buf, size_t *bufsize);
int sc_pkcs15_encode_cdf_entry(struct sc_context *ctx,
			const struct sc_pkcs15_object *obj, u8 **buf,
			size_t *bufsize);
int sc_pkcs15_encode_prkdf_entry(struct sc_context *ctx,
			const struct sc_pkcs15_object *obj, u8 **buf,
			size_t *bufsize);
int sc_pkcs15_encode_pukdf_entry(struct sc_context *ctx,
			const struct sc_pkcs15_object *obj, u8 **buf,
			size_t *bufsize);
int sc_pkcs15_encode_dodf_entry(struct sc_context *ctx,
			const struct sc_pkcs15_object *obj, u8 **buf,
			size_t *bufsize);
int sc_pkcs15_encode_aodf_entry(struct sc_context *ctx,
			const struct sc_pkcs15_object *obj, u8 **buf,
			size_t *bufsize);

int sc_pkcs15_parse_df(struct sc_pkcs15_card *p15card,
		       struct sc_pkcs15_df *df);
int sc_pkcs15_read_df(struct sc_pkcs15_card *p15card,
		      struct sc_pkcs15_df *df);
int sc_pkcs15_decode_cdf_entry(struct sc_pkcs15_card *p15card,
			       struct sc_pkcs15_object *obj,
			       const u8 **buf, size_t *bufsize);
int sc_pkcs15_decode_dodf_entry(struct sc_pkcs15_card *p15card,
			       struct sc_pkcs15_object *obj,
			       const u8 **buf, size_t *bufsize);
int sc_pkcs15_decode_aodf_entry(struct sc_pkcs15_card *p15card,
			        struct sc_pkcs15_object *obj,
			        const u8 **buf, size_t *bufsize);
int sc_pkcs15_decode_prkdf_entry(struct sc_pkcs15_card *p15card,
				 struct sc_pkcs15_object *obj,
				 const u8 **buf, size_t *bufsize);
int sc_pkcs15_decode_pukdf_entry(struct sc_pkcs15_card *p15card,
				 struct sc_pkcs15_object *obj,
				 const u8 **buf, size_t *bufsize);

int sc_pkcs15_decode_enveloped_data(struct sc_context *ctx,
				    struct sc_pkcs15_enveloped_data *result,
				    const u8 *buf, size_t buflen);
int sc_pkcs15_encode_enveloped_data(struct sc_context *ctx,
				    struct sc_pkcs15_enveloped_data *data,
				    u8 **buf, size_t *buflen);

int sc_pkcs15_add_object(struct sc_pkcs15_card *p15card,
			 struct sc_pkcs15_object *obj);
void sc_pkcs15_remove_object(struct sc_pkcs15_card *p15card,
			     struct sc_pkcs15_object *obj);
int sc_pkcs15_add_df(struct sc_pkcs15_card *, unsigned int, const sc_path_t *);
void sc_pkcs15_remove_df(struct sc_pkcs15_card *p15card,
			 struct sc_pkcs15_df *df);

int sc_pkcs15_add_unusedspace(struct sc_pkcs15_card *p15card,
		     const sc_path_t *path, const sc_pkcs15_id_t *auth_id);
void sc_pkcs15_remove_unusedspace(struct sc_pkcs15_card *p15card,
			 sc_pkcs15_unusedspace_t *obj);
int sc_pkcs15_parse_unusedspace(const u8 * buf, size_t buflen,
			struct sc_pkcs15_card *card);
int sc_pkcs15_encode_unusedspace(sc_context_t *ctx,
			 struct sc_pkcs15_card *p15card,
			 u8 **buf, size_t *buflen);

void sc_pkcs15_free_prkey_info(sc_pkcs15_prkey_info_t *key);
void sc_pkcs15_free_pubkey_info(sc_pkcs15_pubkey_info_t *key);
void sc_pkcs15_free_cert_info(sc_pkcs15_cert_info_t *cert);
void sc_pkcs15_free_data_info(sc_pkcs15_data_info_t *data);
void sc_pkcs15_free_auth_info(sc_pkcs15_auth_info_t *auth_info);
void sc_pkcs15_free_object(sc_pkcs15_object_t *obj);

/* Generic file i/o */
int sc_pkcs15_read_file(struct sc_pkcs15_card *p15card,
			const struct sc_path *path,
			u8 **buf, size_t *buflen);

/* Caching functions */
int sc_pkcs15_read_cached_file(struct sc_pkcs15_card *p15card,
                               const struct sc_path *path,
                               u8 **buf, size_t *bufsize);
int sc_pkcs15_cache_file(struct sc_pkcs15_card *p15card,
			 const struct sc_path *path,
			 const u8 *buf, size_t bufsize);

/* PKCS #15 ID handling functions */
int sc_pkcs15_compare_id(const struct sc_pkcs15_id *id1,
			 const struct sc_pkcs15_id *id2);
const char *sc_pkcs15_print_id(const struct sc_pkcs15_id *id);
void sc_pkcs15_format_id(const char *id_in, struct sc_pkcs15_id *id_out);
int sc_pkcs15_hex_string_to_id(const char *in, struct sc_pkcs15_id *out);
int sc_der_copy(sc_pkcs15_der_t *, const sc_pkcs15_der_t *);
int sc_pkcs15_get_object_id(const struct sc_pkcs15_object *, struct sc_pkcs15_id *);
int sc_pkcs15_get_guid(struct sc_pkcs15_card *, const struct sc_pkcs15_object *, 
		char *, size_t);
int sc_encode_oid (struct sc_context *, struct sc_object_id *, 
		unsigned char **, size_t *);

/* Prepend 'parent' to 'child' in case 'child' is a relative path */
int sc_pkcs15_make_absolute_path(const sc_path_t *parent, sc_path_t *child);

/* Clean and free object content */
void sc_pkcs15_free_object_content(struct sc_pkcs15_object *);

/* Allocate and set object content */
int sc_pkcs15_allocate_object_content(struct sc_context *, struct sc_pkcs15_object *,
		const unsigned char *, size_t);

struct sc_supported_algo_info *sc_pkcs15_get_supported_algo(struct sc_pkcs15_card *,
		unsigned, unsigned);
int sc_pkcs15_add_supported_algo_ref(struct sc_pkcs15_object *,
		struct sc_supported_algo_info *);

int sc_pkcs15_fix_ec_parameters(struct sc_context *, struct sc_pkcs15_ec_parameters *);

/* New object search API.
 * More complex, but also more powerful.
 */
typedef struct sc_pkcs15_search_key {
	unsigned int		class_mask;
	unsigned int		type;
	const sc_pkcs15_id_t *	id;
	const struct sc_object_id *app_oid;
	const sc_path_t *	path;
	unsigned int		usage_mask, usage_value;
	unsigned int		flags_mask, flags_value;

	unsigned int		match_reference : 1;
	int			reference;
	const char *		app_label;
	const char *		label;
} sc_pkcs15_search_key_t;

int sc_pkcs15_search_objects(sc_pkcs15_card_t *, sc_pkcs15_search_key_t *,
			sc_pkcs15_object_t **, size_t);

/* This structure is passed to the new sc_pkcs15emu_*_init functions */
typedef struct sc_pkcs15emu_opt {
	scconf_block *blk;
	unsigned int flags;
} sc_pkcs15emu_opt_t;

#define SC_PKCS15EMU_FLAGS_NO_CHECK	0x00000001

extern int sc_pkcs15_bind_synthetic(sc_pkcs15_card_t *);
extern int sc_pkcs15_is_emulation_only(sc_card_t *);

int sc_pkcs15emu_object_add(sc_pkcs15_card_t *, unsigned int,
			const sc_pkcs15_object_t *, const void *);
/* some wrapper functions for sc_pkcs15emu_object_add */
int sc_pkcs15emu_add_pin_obj(sc_pkcs15_card_t *,
	const sc_pkcs15_object_t *, const sc_pkcs15_auth_info_t *);
int sc_pkcs15emu_add_rsa_prkey(sc_pkcs15_card_t *,
	const sc_pkcs15_object_t *, const sc_pkcs15_prkey_info_t *);
int sc_pkcs15emu_add_rsa_pubkey(sc_pkcs15_card_t *,
	const sc_pkcs15_object_t *, const sc_pkcs15_pubkey_info_t *);
int sc_pkcs15emu_add_ec_prkey(sc_pkcs15_card_t *,
	const sc_pkcs15_object_t *, const sc_pkcs15_prkey_info_t *);
int sc_pkcs15emu_add_ec_pubkey(sc_pkcs15_card_t *,
	const sc_pkcs15_object_t *, const sc_pkcs15_pubkey_info_t *);
int sc_pkcs15emu_add_x509_cert(sc_pkcs15_card_t *,
	const sc_pkcs15_object_t *, const sc_pkcs15_cert_info_t *);
int sc_pkcs15emu_add_data_object(sc_pkcs15_card_t *,
	const sc_pkcs15_object_t *, const sc_pkcs15_data_info_t *);

#ifdef __cplusplus
}
#endif

#endif
