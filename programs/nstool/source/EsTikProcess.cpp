#include <iostream>
#include <iomanip>

#include <fnd/SimpleTextOutput.h>
#include "OffsetAdjustedIFile.h"
#include "EsTikProcess.h"



EsTikProcess::EsTikProcess() :
	mFile(nullptr),
	mOwnIFile(false),
	mCliOutputMode(_BIT(OUTPUT_BASIC)),
	mVerify(false)
{
}

EsTikProcess::~EsTikProcess()
{
	if (mOwnIFile)
	{
		delete mFile;
	}
}

void EsTikProcess::process()
{
	fnd::Vec<byte_t> scratch;

	if (mFile == nullptr)
	{
		throw fnd::Exception(kModuleName, "No file reader set.");
	}

	scratch.alloc(mFile->size());
	mFile->read(scratch.data(), 0, scratch.size());

	mTik.fromBytes(scratch.data(), scratch.size());

	if (_HAS_BIT(mCliOutputMode, OUTPUT_BASIC))
		displayTicket();
}

void EsTikProcess::setInputFile(fnd::IFile* file, bool ownIFile)
{
	mFile = file;
	mOwnIFile = ownIFile;
}

void EsTikProcess::setKeyset(const sKeyset* keyset)
{
	mKeyset = keyset;
}

void EsTikProcess::setCliOutputMode(CliOutputMode mode)
{
	mCliOutputMode = mode;
}

void EsTikProcess::setVerifyMode(bool verify)
{
	mVerify = verify;
}

void EsTikProcess::displayTicket()
{
#define _SPLIT_VER(ver) ( (ver>>10) & 0x3f), ( (ver>>4) & 0x3f), ( (ver>>0) & 0xf)
#define _HEXDUMP_U(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02X", var[a__a__A]); } while(0)
#define _HEXDUMP_L(var, len) do { for (size_t a__a__A = 0; a__a__A < len; a__a__A++) printf("%02x", var[a__a__A]); } while(0)

	const es::TicketBody_V2& body = mTik.getBody();	

	std::cout << "[ES Ticket]" << std::endl;

	std::cout << "  SignType:         " << getSignTypeStr(mTik.getSignature().getSignType());
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << " (0x" << std::hex << mTik.getSignature().getSignType() << ")";
	std::cout << std::endl;

	std::cout << "  Issuer:           " << body.getIssuer() << std::endl;
	std::cout << "  Title Key:" << std::endl;
	std::cout << "    EncMode:        " << getTitleKeyPersonalisationStr(body.getTitleKeyEncType()) << std::endl;
	std::cout << "    KeyGeneration:  " << std::dec << (uint32_t)body.getCommonKeyId() << std::endl;
	std::cout << "    Data:" << std::endl;
	size_t size = body.getTitleKeyEncType() == es::ticket::RSA2048 ? crypto::rsa::kRsa2048Size : crypto::aes::kAes128KeySize;
	fnd::SimpleTextOutput::hexDump(body.getEncTitleKey(), size, 0x10, 6);

	printf("  Version:          v%d.%d.%d", _SPLIT_VER(body.getTicketVersion()));
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		printf(" (%d)", body.getTicketVersion());
	printf("\n");
	
	std::cout << "  License Type:     " << getLicenseTypeStr(body.getLicenseType()) << std::endl; 
	
	if (body.getPropertyFlags().size() > 0)
	{
		std::cout << "  Flags:" << std::endl;
		for (size_t i = 0; i < body.getPropertyFlags().size(); i++)
		{
			std::cout << "    " << getPropertyFlagStr(body.getPropertyFlags()[i]) << std::endl;
		}
	}
	
	if (_HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
	{
		std::cout << "  Reserved Region:" << std::endl;
		fnd::SimpleTextOutput::hexDump(body.getReservedRegion(), 8, 0x10, 4);
	}
	
	if (body.getTicketId() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << "  TicketId:         0x" << std::hex << std::setw(16) << std::setfill('0') << body.getTicketId() << std::endl;
	
	if (body.getDeviceId() != 0 || _HAS_BIT(mCliOutputMode, OUTPUT_EXTENDED))
		std::cout << "  DeviceId:         0x" << std::hex << std::setw(16) << std::setfill('0') << body.getDeviceId() << std::endl;
	
	std::cout << "  RightsId:         " <<  std::endl << "    ";
	fnd::SimpleTextOutput::hexDump(body.getRightsId(), 16);

	std::cout << "  SectionTotalSize:       0x" << std::hex << body.getSectionTotalSize() << std::endl;
	std::cout << "  SectionHeaderOffset:    0x" << std::hex << body.getSectionHeaderOffset() << std::endl;
	std::cout << "  SectionNum:             0x" << std::hex << body.getSectionNum() << std::endl;
	std::cout << "  SectionEntrySize:       0x" << std::hex << body.getSectionEntrySize() << std::endl;

	
#undef _HEXDUMP_L
#undef _HEXDUMP_U
#undef _SPLIT_VER
}

const char* EsTikProcess::getSignTypeStr(uint32_t type) const
{
	const char* str = nullptr;
	switch(type)
	{
	case (es::sign::SIGN_ID_RSA4096_SHA1):
		str = "RSA4096-SHA1";
		break;
	case (es::sign::SIGN_ID_RSA2048_SHA1):
		str = "RSA2048-SHA1";
		break;
	case (es::sign::SIGN_ID_ECDSA240_SHA1):
		str = "ECDSA240-SHA1";
		break;
	case (es::sign::SIGN_ID_RSA4096_SHA256):
		str = "RSA4096-SHA256";
		break;
	case (es::sign::SIGN_ID_RSA2048_SHA256):
		str = "RSA2048-SHA256";
		break;
	case (es::sign::SIGN_ID_ECDSA240_SHA256):
		str = "ECDSA240-SHA256";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsTikProcess::getTitleKeyPersonalisationStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (es::ticket::AES128_CBC):
		str = "Generic (AESCBC)";
		break;
	case (es::ticket::RSA2048):
		str = "Personalised (RSA2048)";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsTikProcess::getLicenseTypeStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (es::ticket::LICENSE_PERMANENT):
		str = "Permanent";
		break;
	case (es::ticket::LICENSE_DEMO):
		str = "Demo";
		break;
	case (es::ticket::LICENSE_TRIAL):
		str = "Trial";
		break;
	case (es::ticket::LICENSE_RENTAL):
		str = "Rental";
		break;
	case (es::ticket::LICENSE_SUBSCRIPTION):
		str = "Subscription";
		break;
	case (es::ticket::LICENSE_SERVICE):
		str = "Service";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}

const char* EsTikProcess::getPropertyFlagStr(byte_t flag) const
{
	const char* str = nullptr;
	switch(flag)
	{
	case (es::ticket::FLAG_PRE_INSTALL):
		str = "PreInstall";
		break;
	case (es::ticket::FLAG_SHARED_TITLE):
		str = "SharedTitle";
		break;
	case (es::ticket::FLAG_ALLOW_ALL_CONTENT):
		str = "AllContent";
		break;
	default:
		str = "Unknown";
		break;
	}
	return str;
}