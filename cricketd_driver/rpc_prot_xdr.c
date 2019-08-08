/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "cd_rpc_prot.h"

bool_t
xdr_int_result (XDR *xdrs, int_result *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->err))
		 return FALSE;
	switch (objp->err) {
	case 0:
		 if (!xdr_int (xdrs, &objp->int_result_u.data))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_u64_result (XDR *xdrs, u64_result *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->err))
		 return FALSE;
	switch (objp->err) {
	case 0:
		 if (!xdr_uint64_t (xdrs, &objp->u64_result_u.u64))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_ptr_result (XDR *xdrs, ptr_result *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->err))
		 return FALSE;
	switch (objp->err) {
	case 0:
		 if (!xdr_uint64_t (xdrs, &objp->ptr_result_u.ptr))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_ptr_ptr (XDR *xdrs, ptr_ptr *objp)
{
	register int32_t *buf;

	 if (!xdr_uint64_t (xdrs, &objp->ptr1))
		 return FALSE;
	 if (!xdr_uint64_t (xdrs, &objp->ptr2))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_str_result (XDR *xdrs, str_result *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->err))
		 return FALSE;
	switch (objp->err) {
	case 0:
		 if (!xdr_string (xdrs, &objp->str_result_u.str, 128))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_uuid_result (XDR *xdrs, uuid_result *objp)
{
	register int32_t *buf;

	 if (!xdr_int (xdrs, &objp->err))
		 return FALSE;
	switch (objp->err) {
	case 0:
		 if (!xdr_vector (xdrs, (char *)objp->uuid_result_u.bytes, 16,
			sizeof (char), (xdrproc_t) xdr_char))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_rpc_uuid (XDR *xdrs, rpc_uuid *objp)
{
	register int32_t *buf;

	 if (!xdr_array (xdrs, (char **)&objp->rpc_uuid_val, (u_int *) &objp->rpc_uuid_len, 16,
		sizeof (char), (xdrproc_t) xdr_char))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_mem_data (XDR *xdrs, mem_data *objp)
{
	register int32_t *buf;

	 if (!xdr_bytes (xdrs, (char **)&objp->mem_data_val, (u_int *) &objp->mem_data_len, ~0))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_rpc_cudevicegetattribute_1_argument (XDR *xdrs, rpc_cudevicegetattribute_1_argument *objp)
{
	 if (!xdr_int (xdrs, &objp->arg1))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->arg2))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_rpc_cumodulegetfunction_1_argument (XDR *xdrs, rpc_cumodulegetfunction_1_argument *objp)
{
	 if (!xdr_uint64_t (xdrs, &objp->arg1))
		 return FALSE;
	 if (!xdr_string (xdrs, &objp->arg2, ~0))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_rpc_cumemcpyhtod_1_argument (XDR *xdrs, rpc_cumemcpyhtod_1_argument *objp)
{
	 if (!xdr_uint64_t (xdrs, &objp->arg1))
		 return FALSE;
	 if (!xdr_mem_data (xdrs, &objp->arg2))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_rpc_culaunchkernel_1_argument (XDR *xdrs, rpc_culaunchkernel_1_argument *objp)
{
	 if (!xdr_uint64_t (xdrs, &objp->arg1))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->arg2))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->arg3))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->arg4))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->arg5))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->arg6))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->arg7))
		 return FALSE;
	 if (!xdr_u_int (xdrs, &objp->arg8))
		 return FALSE;
	 if (!xdr_uint64_t (xdrs, &objp->arg9))
		 return FALSE;
	 if (!xdr_mem_data (xdrs, &objp->arg10))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_rpc_hidden_get_module_1_argument (XDR *xdrs, rpc_hidden_get_module_1_argument *objp)
{
	 if (!xdr_uint64_t (xdrs, &objp->arg2))
		 return FALSE;
	 if (!xdr_uint64_t (xdrs, &objp->arg3))
		 return FALSE;
	 if (!xdr_uint64_t (xdrs, &objp->arg4))
		 return FALSE;
	 if (!xdr_int (xdrs, &objp->arg5))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_rpc_hidden_1_3_1_argument (XDR *xdrs, rpc_hidden_1_3_1_argument *objp)
{
	 if (!xdr_uint64_t (xdrs, &objp->arg1))
		 return FALSE;
	 if (!xdr_uint64_t (xdrs, &objp->arg2))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_rpc_hidden_3_0_1_argument (XDR *xdrs, rpc_hidden_3_0_1_argument *objp)
{
	 if (!xdr_int (xdrs, &objp->arg1))
		 return FALSE;
	 if (!xdr_uint64_t (xdrs, &objp->arg2))
		 return FALSE;
	 if (!xdr_uint64_t (xdrs, &objp->arg3))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_rpc_hidden_3_2_1_argument (XDR *xdrs, rpc_hidden_3_2_1_argument *objp)
{
	 if (!xdr_int (xdrs, &objp->arg1))
		 return FALSE;
	 if (!xdr_uint64_t (xdrs, &objp->arg2))
		 return FALSE;
	return TRUE;
}
