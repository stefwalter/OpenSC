/*
 * pkcs15-pin.c: PKCS #15 PIN functions
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

#include "config.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "internal.h"
#include "asn1.h"
#include "pkcs15.h"

static const struct sc_asn1_entry c_asn1_com_ao_attr[] = {
	{ "authId",       SC_ASN1_PKCS15_ID, SC_ASN1_TAG_OCTET_STRING, 0, NULL, NULL },
	{ NULL, 0, 0, 0, NULL, NULL }
};
static const struct sc_asn1_entry c_asn1_pin_attr[] = {
	{ "pinFlags",	  SC_ASN1_BIT_FIELD, SC_ASN1_TAG_BIT_STRING, 0, NULL, NULL },
	{ "pinType",      SC_ASN1_ENUMERATED, SC_ASN1_TAG_ENUMERATED, 0, NULL, NULL },
	{ "minLength",    SC_ASN1_INTEGER, SC_ASN1_TAG_INTEGER, 0, NULL, NULL },
	{ "storedLength", SC_ASN1_INTEGER, SC_ASN1_TAG_INTEGER, 0, NULL, NULL },
	{ "maxLength",    SC_ASN1_INTEGER, SC_ASN1_TAG_INTEGER, SC_ASN1_OPTIONAL, NULL, NULL },
	{ "pinReference", SC_ASN1_INTEGER, SC_ASN1_CTX | 0, SC_ASN1_OPTIONAL, NULL, NULL },
	{ "padChar",      SC_ASN1_OCTET_STRING, SC_ASN1_TAG_OCTET_STRING, SC_ASN1_OPTIONAL, NULL, NULL },
	{ "lastPinChange",SC_ASN1_GENERALIZEDTIME, SC_ASN1_TAG_GENERALIZEDTIME, SC_ASN1_OPTIONAL, NULL, NULL },
	{ "path",         SC_ASN1_PATH, SC_ASN1_TAG_SEQUENCE | SC_ASN1_CONS, SC_ASN1_OPTIONAL, NULL, NULL },
	{ NULL, 0, 0, 0, NULL, NULL }
};
static const struct sc_asn1_entry c_asn1_type_pin_attr[] = {
	{ "pinAttributes", SC_ASN1_STRUCT, SC_ASN1_TAG_SEQUENCE | SC_ASN1_CONS, 0, NULL, NULL },
	{ NULL, 0, 0, 0, NULL, NULL }
};
static const struct sc_asn1_entry c_asn1_pin[] = {
	{ "pin", SC_ASN1_PKCS15_OBJECT, SC_ASN1_TAG_SEQUENCE | SC_ASN1_CONS, 0, NULL, NULL },
	{ NULL, 0, 0, 0, NULL, NULL }
};

int sc_pkcs15_decode_aodf_entry(struct sc_pkcs15_card *p15card,
				struct sc_pkcs15_object *obj,
				const u8 ** buf, size_t *buflen)
{
	sc_context_t *ctx = p15card->card->ctx;
	struct sc_pkcs15_auth_info info;
	int r;
	size_t flags_len = sizeof(info.attrs.pin.flags);
	size_t padchar_len = 1;
	struct sc_asn1_entry asn1_com_ao_attr[2], asn1_pin_attr[10], asn1_type_pin_attr[2];
	struct sc_asn1_entry asn1_pin[2];
	struct sc_asn1_pkcs15_object pin_obj = { obj, asn1_com_ao_attr, NULL, asn1_type_pin_attr };
	
	SC_FUNC_CALLED(ctx, SC_LOG_DEBUG_ASN1);
	sc_copy_asn1_entry(c_asn1_pin, asn1_pin);
	sc_copy_asn1_entry(c_asn1_type_pin_attr, asn1_type_pin_attr);
	sc_copy_asn1_entry(c_asn1_pin_attr, asn1_pin_attr);
	sc_copy_asn1_entry(c_asn1_com_ao_attr, asn1_com_ao_attr);

	sc_format_asn1_entry(asn1_pin + 0, &pin_obj, NULL, 0);

	sc_format_asn1_entry(asn1_type_pin_attr + 0, asn1_pin_attr, NULL, 0);

	sc_format_asn1_entry(asn1_pin_attr + 0, &info.attrs.pin.flags, &flags_len, 0);
	sc_format_asn1_entry(asn1_pin_attr + 1, &info.attrs.pin.type, NULL, 0);
	sc_format_asn1_entry(asn1_pin_attr + 2, &info.attrs.pin.min_length, NULL, 0);
	sc_format_asn1_entry(asn1_pin_attr + 3, &info.attrs.pin.stored_length, NULL, 0);
	sc_format_asn1_entry(asn1_pin_attr + 4, &info.attrs.pin.max_length, NULL, 0);
	sc_format_asn1_entry(asn1_pin_attr + 5, &info.attrs.pin.reference, NULL, 0);
	sc_format_asn1_entry(asn1_pin_attr + 6, &info.attrs.pin.pad_char, &padchar_len, 0);
	/* We don't support lastPinChange yet. */
	sc_format_asn1_entry(asn1_pin_attr + 8, &info.path, NULL, 0);

	sc_format_asn1_entry(asn1_com_ao_attr + 0, &info.auth_id, NULL, 0);

	/* Fill in defaults */
	memset(&info, 0, sizeof(info));
	info.auth_type = SC_PKCS15_PIN_AUTH_TYPE_PIN;
	info.tries_left = -1;

	r = sc_asn1_decode(ctx, asn1_pin, *buf, *buflen, buf, buflen);
	if (r == SC_ERROR_ASN1_END_OF_CONTENTS)
		return r;
	SC_TEST_RET(ctx, SC_LOG_DEBUG_NORMAL, r, "ASN.1 decoding failed");

	obj->type = SC_PKCS15_TYPE_AUTH_PIN;
	obj->data = malloc(sizeof(info));
	if (obj->data == NULL)
		SC_FUNC_RETURN(ctx, SC_LOG_DEBUG_NORMAL, SC_ERROR_OUT_OF_MEMORY);

	if (info.attrs.pin.max_length == 0) {
		if (p15card->card->max_pin_len != 0)
			info.attrs.pin.max_length = p15card->card->max_pin_len;
		else if (info.attrs.pin.stored_length != 0)
			info.attrs.pin.max_length = info.attrs.pin.type != SC_PKCS15_PIN_TYPE_BCD ?
				info.attrs.pin.stored_length : 2 * info.attrs.pin.stored_length;
		else
			info.attrs.pin.max_length = 8; /* shouldn't happen */
	}

	/* OpenSC 0.11.4 and older encoded "pinReference" as a negative
	   value. Fixed in 0.11.5 we need to add a hack, so old cards
	   continue to work. 
	   The same invalid encoding has some models of the proprietary PKCS#15 cards.
	*/
	if (info.attrs.pin.reference < 0)
		info.attrs.pin.reference += 256;

	info.auth_method = SC_AC_CHV;

	if (info.attrs.pin.flags & SC_PKCS15_PIN_FLAG_LOCAL)   {
		/* In OpenSC pkcs#15 framework 'path' is mandatory for the 'Local' PINs. 
		 * If 'path' do not present in PinAttributes, 
		 * 	derive it from the PKCS#15 context. */
		if (!info.path.len)   {
			/* Give priority to AID defined in the application DDO */
			if (p15card->app && p15card->app->ddo.aid.len)
				info.path.aid = p15card->app->ddo.aid;
			else if (p15card->file_app->path.len)
				info.path = p15card->file_app->path;
		}
	}
	sc_debug(ctx, SC_LOG_DEBUG_ASN1, "decoded PIN(ref:%X,path:%s)", info.attrs.pin.reference, sc_print_path(&info.path));

	memcpy(obj->data, &info, sizeof(info));
	SC_FUNC_RETURN(ctx, SC_LOG_DEBUG_ASN1, SC_SUCCESS);
}

int sc_pkcs15_encode_aodf_entry(sc_context_t *ctx,
				 const struct sc_pkcs15_object *obj,
				 u8 **buf, size_t *buflen)
{
	struct sc_asn1_entry asn1_com_ao_attr[2], asn1_pin_attr[10], asn1_type_pin_attr[2];
	struct sc_asn1_entry asn1_pin[2];
	struct sc_pkcs15_auth_info *info = (struct sc_pkcs15_auth_info *) obj->data;
	struct sc_asn1_pkcs15_object pin_obj = { (struct sc_pkcs15_object *) obj,
						 asn1_com_ao_attr, NULL, asn1_type_pin_attr };
	int r;
	size_t flags_len;
	size_t padchar_len = 1;

	if (info->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN)
		return SC_ERROR_NOT_SUPPORTED;

	sc_copy_asn1_entry(c_asn1_pin, asn1_pin);
        sc_copy_asn1_entry(c_asn1_type_pin_attr, asn1_type_pin_attr);
        sc_copy_asn1_entry(c_asn1_pin_attr, asn1_pin_attr);
        sc_copy_asn1_entry(c_asn1_com_ao_attr, asn1_com_ao_attr);

	sc_format_asn1_entry(asn1_pin + 0, &pin_obj, NULL, 1);

	sc_format_asn1_entry(asn1_type_pin_attr + 0, asn1_pin_attr, NULL, 1);

	flags_len = sizeof(info->attrs.pin.flags);
	sc_format_asn1_entry(asn1_pin_attr + 0, &info->attrs.pin.flags, &flags_len, 1);
	sc_format_asn1_entry(asn1_pin_attr + 1, &info->attrs.pin.type, NULL, 1);
	sc_format_asn1_entry(asn1_pin_attr + 2, &info->attrs.pin.min_length, NULL, 1);
	sc_format_asn1_entry(asn1_pin_attr + 3, &info->attrs.pin.stored_length, NULL, 1);
	if (info->attrs.pin.max_length > 0)
		sc_format_asn1_entry(asn1_pin_attr + 4, &info->attrs.pin.max_length, NULL, 1);
	if (info->attrs.pin.reference >= 0)
		sc_format_asn1_entry(asn1_pin_attr + 5, &info->attrs.pin.reference, NULL, 1);
	/* FIXME: check if pad_char present */
	sc_format_asn1_entry(asn1_pin_attr + 6, &info->attrs.pin.pad_char, &padchar_len, 1);
	sc_format_asn1_entry(asn1_pin_attr + 8, &info->path, NULL, info->path.len ? 1 : 0);

	sc_format_asn1_entry(asn1_com_ao_attr + 0, &info->auth_id, NULL, 1);

	r = sc_asn1_encode(ctx, asn1_pin, buf, buflen);

	return r;
}

static int _validate_pin(struct sc_pkcs15_card *p15card,
                         struct sc_pkcs15_auth_info *auth_info,
                         size_t pinlen)
{
	size_t max_length;
	assert(p15card != NULL);

	/* Ignore validation of the non-PIN authentication objects */
	if (auth_info->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN)
		return SC_SUCCESS;

	/* prevent buffer overflow from hostile card */	
	if (auth_info->attrs.pin.stored_length > SC_MAX_PIN_SIZE)
		return SC_ERROR_BUFFER_TOO_SMALL;

	/* if we use pinpad, no more checks are needed */
	if (p15card->card->reader->capabilities & SC_READER_CAP_PIN_PAD)
		return SC_SUCCESS;
		
	/* If pin is given, make sure it is within limits */
	max_length = auth_info->attrs.pin.max_length != 0 ? auth_info->attrs.pin.max_length : SC_MAX_PIN_SIZE;
	if (pinlen > max_length || pinlen < auth_info->attrs.pin.min_length)
		return SC_ERROR_INVALID_PIN_LENGTH;

	return SC_SUCCESS;
}

/*
 * Verify a PIN.
 *
 * If the code given to us has zero length, this means we
 * should ask the card reader to obtain the PIN from the
 * reader's PIN pad
 */
int sc_pkcs15_verify_pin(struct sc_pkcs15_card *p15card,
			 struct sc_pkcs15_object *pin_obj,
			 const unsigned char *pincode, size_t pinlen)
{
	struct sc_context *ctx = p15card->card->ctx;
	struct sc_pkcs15_auth_info *auth_info = (struct sc_pkcs15_auth_info *)pin_obj->data;
	int r;
	sc_card_t *card;
	struct sc_pin_cmd_data data;

	SC_FUNC_CALLED(ctx, SC_LOG_DEBUG_NORMAL);
	sc_debug(ctx, SC_LOG_DEBUG_NORMAL, "PIN(%p;len:%i)", pincode, pinlen);

	/* TODO: verify other authentication objects */
	if (auth_info->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN)
		return SC_ERROR_NOT_SUPPORTED;

	r = _validate_pin(p15card, auth_info, pinlen);
	SC_TEST_RET(ctx, SC_LOG_DEBUG_NORMAL, r, "PIN value do not conforms the PIN policy");

	card = p15card->card;

	r = sc_lock(card);
	SC_TEST_RET(ctx, SC_LOG_DEBUG_NORMAL, r, "sc_lock() failed");
	/* the path in the pin object is optional */
	if (auth_info->path.len > 0) {
		r = sc_select_file(card, &auth_info->path, NULL);
		if (r)
			goto out;
	}

	/* Initialize arguments */
	memset(&data, 0, sizeof(data));
	data.cmd = SC_PIN_CMD_VERIFY;
	data.pin_type = auth_info->auth_method;
	data.pin_reference = auth_info->attrs.pin.reference;
	data.pin1.min_length = auth_info->attrs.pin.min_length;
	data.pin1.max_length = auth_info->attrs.pin.max_length;
	data.pin1.pad_length = auth_info->attrs.pin.stored_length;
	data.pin1.pad_char = auth_info->attrs.pin.pad_char;
	data.pin1.data = pincode;
	data.pin1.len = pinlen;

	if (auth_info->attrs.pin.flags & SC_PKCS15_PIN_FLAG_NEEDS_PADDING)
		data.flags |= SC_PIN_CMD_NEED_PADDING;

	switch (auth_info->attrs.pin.type) {
	case SC_PKCS15_PIN_TYPE_BCD:
		data.pin1.encoding = SC_PIN_ENCODING_BCD;
		break;
	case SC_PKCS15_PIN_TYPE_ASCII_NUMERIC:
		data.pin1.encoding = SC_PIN_ENCODING_ASCII;
		break;
	default:
		/* assume/hope the card driver knows how to encode the pin */
		data.pin1.encoding = 0;
	}

	if(p15card->card->reader->capabilities & SC_READER_CAP_PIN_PAD) {
		if (!pincode && !pinlen)
			data.flags |= SC_PIN_CMD_USE_PINPAD;
		if (auth_info->attrs.pin.flags & SC_PKCS15_PIN_FLAG_SO_PIN)
			data.pin1.prompt = "Please enter SO PIN";
		else
			data.pin1.prompt = "Please enter PIN";
	}

	r = sc_pin_cmd(card, &data, &auth_info->tries_left);
	if (r == SC_SUCCESS)
		sc_pkcs15_pincache_add(p15card, pin_obj, pincode, pinlen);
out:
	sc_unlock(card);
	SC_FUNC_RETURN(ctx, SC_LOG_DEBUG_NORMAL, r);
}

/*
 * Change a PIN.
 */
int sc_pkcs15_change_pin(struct sc_pkcs15_card *p15card,
			 struct sc_pkcs15_object *pin_obj,
			 const u8 *oldpin, size_t oldpinlen,
			 const u8 *newpin, size_t newpinlen)
{
	int r;
	sc_card_t *card;
	struct sc_pin_cmd_data data;
	struct sc_pkcs15_auth_info *auth_info = (struct sc_pkcs15_auth_info *)pin_obj->data;
	
	if (auth_info->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN)
		return SC_ERROR_NOT_SUPPORTED;

	/* make sure the pins are in valid range */
	if ((r = _validate_pin(p15card, auth_info, oldpinlen)) != SC_SUCCESS)
		return r;
	if ((r = _validate_pin(p15card, auth_info, newpinlen)) != SC_SUCCESS)
		return r;

	card = p15card->card;
	r = sc_lock(card);
	SC_TEST_RET(card->ctx, SC_LOG_DEBUG_NORMAL, r, "sc_lock() failed");
	/* the path in the pin object is optional */
	if (auth_info->path.len > 0) {
		r = sc_select_file(card, &auth_info->path, NULL);
		if (r)
			goto out;
	}

	/* set pin_cmd data */
	memset(&data, 0, sizeof(data));
	data.cmd             = SC_PIN_CMD_CHANGE;
	data.pin_type        = SC_AC_CHV;
	data.pin_reference   = auth_info->attrs.pin.reference;
	data.pin1.data       = oldpin;
	data.pin1.len        = oldpinlen;
	data.pin1.pad_char   = auth_info->attrs.pin.pad_char;
	data.pin1.min_length = auth_info->attrs.pin.min_length;
	data.pin1.max_length = auth_info->attrs.pin.max_length;
	data.pin1.pad_length = auth_info->attrs.pin.stored_length;
	data.pin2.data       = newpin;
	data.pin2.len        = newpinlen;
	data.pin2.pad_char   = auth_info->attrs.pin.pad_char;
	data.pin2.min_length = auth_info->attrs.pin.min_length;
	data.pin2.max_length = auth_info->attrs.pin.max_length;
	data.pin2.pad_length = auth_info->attrs.pin.stored_length;

	if (auth_info->attrs.pin.flags & SC_PKCS15_PIN_FLAG_NEEDS_PADDING)
		data.flags |= SC_PIN_CMD_NEED_PADDING;

	switch (auth_info->attrs.pin.type) {
	case SC_PKCS15_PIN_TYPE_BCD:
		data.pin1.encoding = SC_PIN_ENCODING_BCD;
		data.pin2.encoding = SC_PIN_ENCODING_BCD;
		break;
	case SC_PKCS15_PIN_TYPE_ASCII_NUMERIC:
		data.pin1.encoding = SC_PIN_ENCODING_ASCII;
		data.pin2.encoding = SC_PIN_ENCODING_ASCII;
		break;
	}
	
	if((!oldpin || !newpin) 
			&& p15card->card->reader->capabilities & SC_READER_CAP_PIN_PAD) {
		data.flags |= SC_PIN_CMD_USE_PINPAD;
		if (auth_info->attrs.pin.flags & SC_PKCS15_PIN_FLAG_SO_PIN) {
			data.pin1.prompt = "Please enter SO PIN";
			data.pin2.prompt = "Please enter new SO PIN";
		} else {
			data.pin1.prompt = "Please enter PIN";
			data.pin2.prompt = "Please enter new PIN";
		}
	}

	r = sc_pin_cmd(card, &data, &auth_info->tries_left);
	if (r == SC_SUCCESS)
		sc_pkcs15_pincache_add(p15card, pin_obj, newpin, newpinlen);

out:
	sc_unlock(card);
	return r;
}

/*
 * Unblock a PIN.
 */
int sc_pkcs15_unblock_pin(struct sc_pkcs15_card *p15card,
			 struct sc_pkcs15_object *pin_obj,
			 const u8 *puk, size_t puklen,
			 const u8 *newpin, size_t newpinlen)
{
	int r;
	sc_card_t *card;
	struct sc_pin_cmd_data data;
	struct sc_pkcs15_object *puk_obj;
	struct sc_pkcs15_auth_info *puk_info = NULL;
	struct sc_pkcs15_auth_info *auth_info = (struct sc_pkcs15_auth_info *)pin_obj->data;

	if (auth_info->auth_type != SC_PKCS15_PIN_AUTH_TYPE_PIN)
		return SC_ERROR_NOT_SUPPORTED;

	/* make sure the pins are in valid range */
	if ((r = _validate_pin(p15card, auth_info, newpinlen)) != SC_SUCCESS)
		return r;

	card = p15card->card;
	/* get pin_info object of the puk (this is a little bit complicated
	 * as we don't have the id of the puk (at least now))
	 * note: for compatibility reasons we give no error if no puk object
	 * is found */
	/* first step: try to get the pkcs15 object of the puk */
	r = sc_pkcs15_find_pin_by_auth_id(p15card, &pin_obj->auth_id, &puk_obj);
	if (r >= 0 && puk_obj) {
		/* second step:  get the pkcs15 info object of the puk */
		puk_info = (struct sc_pkcs15_auth_info *)puk_obj->data;
	}
	if (!puk_info) {
		sc_debug(card->ctx, SC_LOG_DEBUG_NORMAL, "Unable to get puk object, using pin object instead!");
		puk_info = auth_info;
	}
	
	/* make sure the puk is in valid range */
	if ((r = _validate_pin(p15card, puk_info, puklen)) != SC_SUCCESS)
		return r;

	r = sc_lock(card);
	SC_TEST_RET(card->ctx, SC_LOG_DEBUG_NORMAL, r, "sc_lock() failed");
	/* the path in the pin object is optional */
	if (auth_info->path.len > 0) {
		r = sc_select_file(card, &auth_info->path, NULL);
		if (r)
			goto out;
	}

	/* set pin_cmd data */
	memset(&data, 0, sizeof(data));
	data.cmd             = SC_PIN_CMD_UNBLOCK;
	data.pin_type        = SC_AC_CHV;
	data.pin_reference   = auth_info->attrs.pin.reference;
	data.pin1.data       = puk;
	data.pin1.len        = puklen;
	data.pin1.pad_char   = auth_info->attrs.pin.pad_char;
	data.pin1.min_length = auth_info->attrs.pin.min_length;
	data.pin1.max_length = auth_info->attrs.pin.max_length;
	data.pin1.pad_length = auth_info->attrs.pin.stored_length;
	data.pin2.data       = newpin;
	data.pin2.len        = newpinlen;
	data.pin2.pad_char   = puk_info->attrs.pin.pad_char;
	data.pin2.min_length = puk_info->attrs.pin.min_length;
	data.pin2.max_length = puk_info->attrs.pin.max_length;
	data.pin2.pad_length = puk_info->attrs.pin.stored_length;

	if (auth_info->attrs.pin.flags & SC_PKCS15_PIN_FLAG_NEEDS_PADDING)
		data.flags |= SC_PIN_CMD_NEED_PADDING;

	switch (auth_info->attrs.pin.type) {
	case SC_PKCS15_PIN_TYPE_BCD:
		data.pin1.encoding = SC_PIN_ENCODING_BCD;
		break;
	case SC_PKCS15_PIN_TYPE_ASCII_NUMERIC:
		data.pin1.encoding = SC_PIN_ENCODING_ASCII;
		break;
	}

	switch (puk_info->attrs.pin.type) {
	case SC_PKCS15_PIN_TYPE_BCD:
		data.pin2.encoding = SC_PIN_ENCODING_BCD;
		break;
	case SC_PKCS15_PIN_TYPE_ASCII_NUMERIC:
		data.pin2.encoding = SC_PIN_ENCODING_ASCII;
		break;
	}
	
	if(p15card->card->reader->capabilities & SC_READER_CAP_PIN_PAD) {
		data.flags |= SC_PIN_CMD_USE_PINPAD;
		if (auth_info->attrs.pin.flags & SC_PKCS15_PIN_FLAG_SO_PIN) {
			data.pin1.prompt = "Please enter PUK";
			data.pin2.prompt = "Please enter new SO PIN";
		} else {
			data.pin1.prompt = "Please enter PUK";
			data.pin2.prompt = "Please enter new PIN";
		}
	}

	r = sc_pin_cmd(card, &data, &auth_info->tries_left);
	if (r == SC_SUCCESS)
		sc_pkcs15_pincache_add(p15card, pin_obj, newpin, newpinlen);

out:
	sc_unlock(card);
	return r;
}

void sc_pkcs15_free_auth_info(sc_pkcs15_auth_info_t *auth_info)
{
	free(auth_info);
}


/* Add a PIN to the PIN cache related to the card. Some operations can trigger re-authentication later. */
void sc_pkcs15_pincache_add(struct sc_pkcs15_card *p15card, struct sc_pkcs15_object *pin_obj,
	const u8 *pin, size_t pinlen)
{
	struct sc_context *ctx = p15card->card->ctx;
	struct sc_pkcs15_auth_info *auth_info = (struct sc_pkcs15_auth_info *)pin_obj->data;
	struct sc_pkcs15_object *obj = NULL;
	int r;

	SC_FUNC_CALLED(ctx, SC_LOG_DEBUG_NORMAL);

	if (!p15card->opts.use_pin_cache)   {
		sc_debug(ctx, SC_LOG_DEBUG_NORMAL, "PIN caching not enabled");
		return;
	}

	/* If the PIN protects an object with user consent, don't cache it */

	obj = p15card->obj_list;
	while (obj != NULL) {
		/* Compare 'sc_pkcs15_object.auth_id' with 'sc_pkcs15_pin_info.auth_id'.
		 * In accordance with PKCS#15 "6.1.8 CommonObjectAttributes" and
		 * "6.1.16 CommonAuthenticationObjectAttributes" with the exception that
		 * "CommonObjectAttributes.accessControlRules" are not taken into account. */

		if (sc_pkcs15_compare_id(&obj->auth_id, &auth_info->auth_id)) {
			/* Caching is refused, if the protected object requires user consent */
			if (obj->user_consent > 0) {
				sc_debug(ctx, SC_LOG_DEBUG_NORMAL, "caching refused (user consent)");
				return;
			}
		}

		obj = obj->next;
	}

	r = sc_pkcs15_allocate_object_content(ctx, pin_obj, pin, pinlen);
	if (r != SC_SUCCESS)   {
		sc_debug(ctx, SC_LOG_DEBUG_NORMAL, "Failed to allocate object content");
		return;
	} 

	pin_obj->usage_counter = 0;
	sc_debug(ctx, SC_LOG_DEBUG_NORMAL, "PIN(%s) cached", pin_obj->label);
}

/* Validate the PIN code associated with an object */
int sc_pkcs15_pincache_revalidate(struct sc_pkcs15_card *p15card, const sc_pkcs15_object_t *obj)
{
	struct sc_context *ctx = p15card->card->ctx;
	sc_pkcs15_object_t *pin_obj;
	int r;

	SC_FUNC_CALLED(ctx, SC_LOG_DEBUG_NORMAL);

	if (!p15card->opts.use_pin_cache)
		return SC_ERROR_SECURITY_STATUS_NOT_SATISFIED;

	if (obj->user_consent)
		return SC_ERROR_SECURITY_STATUS_NOT_SATISFIED;

	if (p15card->card->reader->capabilities & SC_READER_CAP_PIN_PAD)
		return SC_ERROR_SECURITY_STATUS_NOT_SATISFIED;

	r = sc_pkcs15_find_pin_by_auth_id(p15card, &obj->auth_id, &pin_obj);
	if (r != SC_SUCCESS) {
		sc_debug(ctx, SC_LOG_DEBUG_NORMAL, "Could not find pin object for auth_id %s", sc_pkcs15_print_id(&obj->auth_id));
		return SC_ERROR_SECURITY_STATUS_NOT_SATISFIED;
	}
	
	if (pin_obj->usage_counter >= p15card->opts.pin_cache_counter) {
		sc_pkcs15_free_object_content(pin_obj);
		return SC_ERROR_SECURITY_STATUS_NOT_SATISFIED;
	}

	if (!pin_obj->content.value || !pin_obj->content.len)
		return SC_ERROR_SECURITY_STATUS_NOT_SATISFIED;

	pin_obj->usage_counter++;
	r = sc_pkcs15_verify_pin(p15card, pin_obj, pin_obj->content.value, pin_obj->content.len);
	if (r != SC_SUCCESS) {
		/* Ensure that wrong PIN isn't used again */ 
		sc_pkcs15_free_object_content(pin_obj);

		sc_debug(ctx, SC_LOG_DEBUG_NORMAL, "Verify PIN error %i", r);
		return SC_ERROR_SECURITY_STATUS_NOT_SATISFIED;
	}

	SC_FUNC_RETURN(ctx, SC_LOG_DEBUG_VERBOSE, SC_SUCCESS);
}

void sc_pkcs15_pincache_clear(struct sc_pkcs15_card *p15card)
{
	struct sc_pkcs15_object *objs[32];
	int i, r;

	SC_FUNC_CALLED(p15card->card->ctx, SC_LOG_DEBUG_NORMAL);
	r = sc_pkcs15_get_objects(p15card, SC_PKCS15_TYPE_AUTH_PIN, objs, 32);
	for (i = 0; i < r; i++)
		sc_pkcs15_free_object_content(objs[i]);
}

