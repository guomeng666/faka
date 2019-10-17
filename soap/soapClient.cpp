/* soapClient.cpp
   Generated by gSOAP 2.8.59 for mySap.h

gSOAP XML Web services tools
Copyright (C) 2000-2017, Robert van Engelen, Genivia Inc. All Rights Reserved.
The soapcpp2 tool and its generated software are released under the GPL.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
--------------------------------------------------------------------------------
A commercial use license is available from Genivia Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"

SOAP_SOURCE_STAMP("@(#) soapClient.cpp ver 2.8.59 2019-06-20 03:04:19 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__updatePassword(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__updatePassword *web__updatePassword_, web__updatePasswordResponse &web__updatePasswordResponse_)
{	struct __web__updatePassword soap_tmp___web__updatePassword;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/updatePasswordRequest";
	soap_tmp___web__updatePassword.web__updatePassword_ = web__updatePassword_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__updatePassword(soap, &soap_tmp___web__updatePassword);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__updatePassword(soap, &soap_tmp___web__updatePassword, "-web:updatePassword", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__updatePassword(soap, &soap_tmp___web__updatePassword, "-web:updatePassword", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__updatePasswordResponse*>(&web__updatePasswordResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__updatePasswordResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__updatePasswordResponse_.soap_get(soap, "web:updatePasswordResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__saveVehicleInfo(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__saveVehicleInfo *web__saveVehicleInfo_, web__saveVehicleInfoResponse &web__saveVehicleInfoResponse_)
{	struct __web__saveVehicleInfo soap_tmp___web__saveVehicleInfo;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/saveVehicleInfoRequest";
	soap_tmp___web__saveVehicleInfo.web__saveVehicleInfo_ = web__saveVehicleInfo_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__saveVehicleInfo(soap, &soap_tmp___web__saveVehicleInfo);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__saveVehicleInfo(soap, &soap_tmp___web__saveVehicleInfo, "-web:saveVehicleInfo", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__saveVehicleInfo(soap, &soap_tmp___web__saveVehicleInfo, "-web:saveVehicleInfo", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__saveVehicleInfoResponse*>(&web__saveVehicleInfoResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__saveVehicleInfoResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__saveVehicleInfoResponse_.soap_get(soap, "web:saveVehicleInfoResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__saveGrainUser(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__saveGrainUser *web__saveGrainUser_, web__saveGrainUserResponse &web__saveGrainUserResponse_)
{	struct __web__saveGrainUser soap_tmp___web__saveGrainUser;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/saveGrainUserRequest";
	soap_tmp___web__saveGrainUser.web__saveGrainUser_ = web__saveGrainUser_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__saveGrainUser(soap, &soap_tmp___web__saveGrainUser);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__saveGrainUser(soap, &soap_tmp___web__saveGrainUser, "-web:saveGrainUser", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__saveGrainUser(soap, &soap_tmp___web__saveGrainUser, "-web:saveGrainUser", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__saveGrainUserResponse*>(&web__saveGrainUserResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__saveGrainUserResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__saveGrainUserResponse_.soap_get(soap, "web:saveGrainUserResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__savePoundInfoA(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__savePoundInfoA *web__savePoundInfoA_, web__savePoundInfoAResponse &web__savePoundInfoAResponse_)
{	struct __web__savePoundInfoA soap_tmp___web__savePoundInfoA;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/savePoundInfoARequest";
	soap_tmp___web__savePoundInfoA.web__savePoundInfoA_ = web__savePoundInfoA_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__savePoundInfoA(soap, &soap_tmp___web__savePoundInfoA);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__savePoundInfoA(soap, &soap_tmp___web__savePoundInfoA, "-web:savePoundInfoA", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__savePoundInfoA(soap, &soap_tmp___web__savePoundInfoA, "-web:savePoundInfoA", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__savePoundInfoAResponse*>(&web__savePoundInfoAResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__savePoundInfoAResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__savePoundInfoAResponse_.soap_get(soap, "web:savePoundInfoAResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__savePoundInfo(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__savePoundInfo *web__savePoundInfo_, web__savePoundInfoResponse &web__savePoundInfoResponse_)
{	struct __web__savePoundInfo soap_tmp___web__savePoundInfo;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/savePoundInfoRequest";
	soap_tmp___web__savePoundInfo.web__savePoundInfo_ = web__savePoundInfo_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__savePoundInfo(soap, &soap_tmp___web__savePoundInfo);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__savePoundInfo(soap, &soap_tmp___web__savePoundInfo, "-web:savePoundInfo", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__savePoundInfo(soap, &soap_tmp___web__savePoundInfo, "-web:savePoundInfo", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__savePoundInfoResponse*>(&web__savePoundInfoResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__savePoundInfoResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__savePoundInfoResponse_.soap_get(soap, "web:savePoundInfoResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__saveGPOrders(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__saveGPOrders *web__saveGPOrders_, web__saveGPOrdersResponse &web__saveGPOrdersResponse_)
{	struct __web__saveGPOrders soap_tmp___web__saveGPOrders;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/saveGPOrdersRequest";
	soap_tmp___web__saveGPOrders.web__saveGPOrders_ = web__saveGPOrders_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__saveGPOrders(soap, &soap_tmp___web__saveGPOrders);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__saveGPOrders(soap, &soap_tmp___web__saveGPOrders, "-web:saveGPOrders", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__saveGPOrders(soap, &soap_tmp___web__saveGPOrders, "-web:saveGPOrders", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__saveGPOrdersResponse*>(&web__saveGPOrdersResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__saveGPOrdersResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__saveGPOrdersResponse_.soap_get(soap, "web:saveGPOrdersResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__saveSamplers(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__saveSamplers *web__saveSamplers_, web__saveSamplersResponse &web__saveSamplersResponse_)
{	struct __web__saveSamplers soap_tmp___web__saveSamplers;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/saveSamplersRequest";
	soap_tmp___web__saveSamplers.web__saveSamplers_ = web__saveSamplers_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__saveSamplers(soap, &soap_tmp___web__saveSamplers);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__saveSamplers(soap, &soap_tmp___web__saveSamplers, "-web:saveSamplers", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__saveSamplers(soap, &soap_tmp___web__saveSamplers, "-web:saveSamplers", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__saveSamplersResponse*>(&web__saveSamplersResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__saveSamplersResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__saveSamplersResponse_.soap_get(soap, "web:saveSamplersResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__saveCheckingInfo(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__saveCheckingInfo *web__saveCheckingInfo_, web__saveCheckingInfoResponse &web__saveCheckingInfoResponse_)
{	struct __web__saveCheckingInfo soap_tmp___web__saveCheckingInfo;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/saveCheckingInfoRequest";
	soap_tmp___web__saveCheckingInfo.web__saveCheckingInfo_ = web__saveCheckingInfo_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__saveCheckingInfo(soap, &soap_tmp___web__saveCheckingInfo);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__saveCheckingInfo(soap, &soap_tmp___web__saveCheckingInfo, "-web:saveCheckingInfo", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__saveCheckingInfo(soap, &soap_tmp___web__saveCheckingInfo, "-web:saveCheckingInfo", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__saveCheckingInfoResponse*>(&web__saveCheckingInfoResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__saveCheckingInfoResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__saveCheckingInfoResponse_.soap_get(soap, "web:saveCheckingInfoResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call___web__login(struct soap *soap, const char *soap_endpoint, const char *soap_action, web__login *web__login_, web__loginResponse &web__loginResponse_)
{	struct __web__login soap_tmp___web__login;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://192.168.1.105:8081/fYService";
	if (soap_action == NULL)
		soap_action = "http://webservice.fy.ggzn.org/FYService/loginRequest";
	soap_tmp___web__login.web__login_ = web__login_;
	soap_begin(soap);
	soap->encodingStyle = NULL;
	soap_serializeheader(soap);
	soap_serialize___web__login(soap, &soap_tmp___web__login);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put___web__login(soap, &soap_tmp___web__login, "-web:login", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put___web__login(soap, &soap_tmp___web__login, "-web:login", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!static_cast<web__loginResponse*>(&web__loginResponse_)) // NULL ref?
		return soap_closesock(soap);
	web__loginResponse_.soap_default(soap);
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	web__loginResponse_.soap_get(soap, "web:loginResponse", NULL);
	if (soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	return soap_closesock(soap);
}

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapClient.cpp */
