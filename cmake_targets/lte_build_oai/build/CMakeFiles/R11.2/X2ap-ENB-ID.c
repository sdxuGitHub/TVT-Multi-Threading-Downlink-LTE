/*
 * Generated by asn1c-0.9.24 (http://lionet.info/asn1c)
 * From ASN.1 module "X2AP-IEs"
 * 	found in "/home/isip/Desktop/TEST/mix_DL_UL/openairinterface5g/openair2/X2AP/MESSAGES/ASN1/R11.2/X2AP-IEs.asn"
 * 	`asn1c -gen-PER`
 */

#include "X2ap-ENB-ID.h"

static int
memb_macro_eNB_ID_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const BIT_STRING_t *st = (const BIT_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	if(st->size > 0) {
		/* Size in bits */
		size = 8 * st->size - (st->bits_unused & 0x07);
	} else {
		size = 0;
	}
	
	if((size == 20)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static int
memb_home_eNB_ID_constraint_1(asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	const BIT_STRING_t *st = (const BIT_STRING_t *)sptr;
	size_t size;
	
	if(!sptr) {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	if(st->size > 0) {
		/* Size in bits */
		size = 8 * st->size - (st->bits_unused & 0x07);
	} else {
		size = 0;
	}
	
	if((size == 28)) {
		/* Constraint check succeeded */
		return 0;
	} else {
		_ASN_CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_per_constraints_t asn_PER_memb_macro_eNB_ID_constr_2 GCC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 0,  0,  20,  20 }	/* (SIZE(20..20)) */,
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_home_eNB_ID_constr_3 GCC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 0,  0,  28,  28 }	/* (SIZE(28..28)) */,
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_type_X2ap_ENB_ID_constr_1 GCC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  1,  1,  0,  1 }	/* (0..1,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_X2ap_ENB_ID_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct X2ap_ENB_ID, choice.macro_eNB_ID),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BIT_STRING,
		memb_macro_eNB_ID_constraint_1,
		&asn_PER_memb_macro_eNB_ID_constr_2,
		0,
		"macro-eNB-ID"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct X2ap_ENB_ID, choice.home_eNB_ID),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BIT_STRING,
		memb_home_eNB_ID_constraint_1,
		&asn_PER_memb_home_eNB_ID_constr_3,
		0,
		"home-eNB-ID"
		},
};
static asn_TYPE_tag2member_t asn_MAP_X2ap_ENB_ID_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* macro-eNB-ID at 477 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* home-eNB-ID at 478 */
};
static asn_CHOICE_specifics_t asn_SPC_X2ap_ENB_ID_specs_1 = {
	sizeof(struct X2ap_ENB_ID),
	offsetof(struct X2ap_ENB_ID, _asn_ctx),
	offsetof(struct X2ap_ENB_ID, present),
	sizeof(((struct X2ap_ENB_ID *)0)->present),
	asn_MAP_X2ap_ENB_ID_tag2el_1,
	2,	/* Count of tags in the map */
	0,
	2	/* Extensions start */
};
asn_TYPE_descriptor_t asn_DEF_X2ap_ENB_ID = {
	"X2ap-ENB-ID",
	"X2ap-ENB-ID",
	CHOICE_free,
	CHOICE_print,
	CHOICE_constraint,
	CHOICE_decode_ber,
	CHOICE_encode_der,
	CHOICE_decode_xer,
	CHOICE_encode_xer,
	CHOICE_decode_uper,
	CHOICE_encode_uper,
	CHOICE_decode_aper,
	CHOICE_encode_aper,
	CHOICE_compare,
	CHOICE_outmost_tag,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	&asn_PER_type_X2ap_ENB_ID_constr_1,
	asn_MBR_X2ap_ENB_ID_1,
	2,	/* Elements count */
	&asn_SPC_X2ap_ENB_ID_specs_1	/* Additional specs */
};

