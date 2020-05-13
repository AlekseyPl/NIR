#include "Lte/Core/Uplink/SibAbonParser.h"
#include <string>
#include <System/DebugInfo.h>

namespace Lte {

SibAbonParser::SibAbonParser( ):
	debug(System::DebugInfo::Locate())
{
	parserMap[baseInfo] = [this](const uint8_t *ptr, uint32_t bitSize){this->ParseInfoBS(ptr,bitSize);};
	parserMap[abonInfo] = [this](const uint8_t *ptr, uint32_t bitSize){this->ParseInfoMT(ptr,bitSize);};
}

SibAbonParser::~SibAbonParser()
{

}

void	SibAbonParser::ParseMessage(const uint8_t *ptr, uint32_t bitSize)
{
	const InfoParser* messageType = reinterpret_cast<const InfoParser*>(ptr);

	if(messageType->baseMsg) parserMap[baseInfo](ptr,bitSize);
	else if(messageType->abonMsg) parserMap[abonInfo](ptr,bitSize);
	else 	debug.SendText("Something strange... %#x", *ptr);
}

void	SibAbonParser::ParseInfoBS(const uint8_t *ptr, uint32_t bitSize)
{
	const BaseInfo* bsInfo = reinterpret_cast<const BaseInfo*>(ptr);

	uint32_t sfn = 0;
	reverser.Do(&bsInfo->SFN, &sfn, BitReverser::int32);

	uint32_t syncSfn = 0;
	reverser.Do(&bsInfo->syncSFNoffset, &syncSfn, BitReverser::int16);

	uint8_t	standart = 0;
	std::string standardStr;
	reverser.Do(&bsInfo->bsType,&standart,BitReverser::int8);
	switch( standart) {
	case 1:
		standardStr = "UMTS";
		break;
	case 2:
		standardStr = "GSM";
		break;
	case 3:
		standardStr = "LTE";
		BaseInfo::BS::LteBaseInfo base;
		const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&bsInfo->bs.lteBaseInfo);
		uint8_t* reversePtr = reinterpret_cast<uint8_t*>(&base);

		for( uint32_t t = 0; t < sizeof(BaseInfo::BS::LteBaseInfo); ++t)
			reverser.Do(&ptr[t], &reversePtr[t], BitReverser::int8);
		break;
	}
	uint16_t	ARFCN = 0;
	reverser.Do(&bsInfo->ARFCN,&ARFCN,BitReverser::int16);
	debug.SendText("BS info: SFN %d, syncSFN %d, ARFCN %d, standard %s", sfn, syncSfn, ARFCN, standardStr.c_str() );

}

void	SibAbonParser::ParseInfoMT(const uint8_t *ptr, uint32_t bitSize)
{
	const	AbonInfo* msInfo = reinterpret_cast<const AbonInfo*>( ptr);
	const uint8_t* uptr = reinterpret_cast<const uint8_t*>(ptr);
	for( uint32_t i = 0;i < bitSize; ++i) {
		debug.SendText("pos%d %#x",i,uptr[i]);
	}

	debug.SendText("Abon INFO Active %d", msInfo->stateActive);
	if( msInfo->stateActive ) {
		uint16_t	ARFCN = 0;
		reverser.Do(&msInfo->ARFCN,&ARFCN,BitReverser::int16);
		uint64_t IMSIrev;
		uint64_t IMSIfrom = msInfo->IMSI;
		reverser.Do(&msInfo->IMSI, &IMSIrev, BitReverser::int64 );
		debug.SendText("IMSI FROM %ju hex %#x IMSI %llu hex %#x",IMSIfrom, IMSIfrom, IMSIrev, IMSIrev) ;

		double from = *(reinterpret_cast<double*>(msInfo->IMSI));
		double after = *(reinterpret_cast<double*>(IMSIrev));
		debug.SendText("double IMSI FROM %f IMSI %f",from, after) ;

		uint8_t bsTypeMSG = msInfo->bsType.standard;
		uint8_t	bsType = 0;
		reverser.Do(&msInfo->bsType,&bsType,BitReverser::int8);
		debug.SendText("STANDARD %d ARFCN %d", bsType, ARFCN );
		switch (bsType) {
		case 1:
			uint16_t SC;
			reverser.Do(&msInfo->standartId.scramblingCode, &SC, BitReverser::int16);
			debug.SendText("UMTS SC %d",SC);
			break;
		case 2:
			uint16_t chanID;
			reverser.Do(&msInfo->standartId.chanId, &chanID, BitReverser::int16);
			debug.SendText("GSM channel ID %d",chanID);
			break;
		case 3:

			uint16_t CRNTI;
			reverser.Do(&msInfo->standartId.CRNTI, &CRNTI, BitReverser::int16);
			debug.SendText("LTE CRNTI %d",msInfo->standartId.CRNTI);
			break;
		default:
			break;
		}
	}
}


SibAbonParser::BitReverser::BitReverser( ) :
	bitReverse({
		0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
	})
{
	reverseMap[int8] = [this](const void* in, void* out){this->Reverse8(in,out);};
	reverseMap[int16] = [this](const void* in, void* out){this->Reverse16(in,out);};
	reverseMap[int32] = [this](const void* in, void* out){this->Reverse32(in,out);};
	reverseMap[int64] = [this](const void* in, void* out){this->Reverse64(in,out);};
}


void SibAbonParser::BitReverser::Do(const void *in, void *out, Width width)
{
	reverseMap[width](in,out);
}

void SibAbonParser::BitReverser::Reverse8(const void *in, void *out)
{
	const uint8_t* inPtr = reinterpret_cast<const uint8_t*>(in);
	uint8_t* outPtr = reinterpret_cast<uint8_t*>(out);
	outPtr[0] = bitReverse[inPtr[0]];
}

void SibAbonParser::BitReverser::Reverse16(const void *in, void *out)
{
	const uint8_t* inPtr = reinterpret_cast<const uint8_t*>(in);
	uint8_t* outPtr = reinterpret_cast<uint8_t*>(out);
	for( uint32_t b = 0, r = sizeof(uint16_t)-1; b < sizeof(uint16_t); ++b, --r)
		outPtr[b] = bitReverse[inPtr[r]];
}

void SibAbonParser::BitReverser::Reverse32(const void *in, void *out)
{
	const uint8_t* inPtr = reinterpret_cast<const uint8_t*>(in);
	uint8_t* outPtr = reinterpret_cast<uint8_t*>(out);
	for( uint32_t b = 0, r = sizeof(uint32_t)-1; b < sizeof(uint32_t); ++b, --r)
		outPtr[b] = bitReverse[inPtr[r]];
}

void SibAbonParser::BitReverser::Reverse64(const void *in, void *out)
{
	const uint8_t* inPtr = reinterpret_cast<const uint8_t*>(in);
	uint8_t* outPtr = reinterpret_cast<uint8_t*>(out);
	for( uint32_t b = 0, r = sizeof(uint64_t)-1; b < sizeof(uint64_t); ++b, --r)
		outPtr[b] = bitReverse[inPtr[r]];
}

}
