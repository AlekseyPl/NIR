
#ifndef LTE_SIBABONPARSER_H_
#define LTE_SIBABONPARSER_H_

#include <stdint.h>
#include <unordered_map>
#include <functional>
#include <array>

namespace System {
    class DebugInfo;
}

namespace Lte {

class SibAbonParser {
public:
    SibAbonParser();
    ~SibAbonParser();

    void ParseMessage(const uint8_t* ptr, uint32_t bitSize);

private:

#pragma pack(push,1)
    struct InfoParser {
        uint8_t dummy   : 6;
        uint8_t abonMsg : 1;
        uint8_t baseMsg : 1;
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct BaseInfo : public InfoParser {
        // DownLink system frame number
        uint32_t	SFN;
        // Offset from sync SFN to DL sfn
        uint16_t	syncSFNoffset;
        //
        struct {
            uint8_t     dymmy    : 5;
			uint8_t		standard : 3;
        }			bsType;
        uint16_t	ARFCN;
        union BS {
            uint16_t	umtsScramblingCode;
            struct LteBaseInfo {
                uint8_t nRbIndex :3;
                uint8_t cp : 1;
                uint16_t nCellId : 9;
                uint8_t	duplex : 1;
                uint8_t sfconfig : 5;
                uint8_t sfAssign : 1;
                uint8_t	specialSfPattert : 4;
            };
            LteBaseInfo lteBaseInfo;
        } bs;

    };
#pragma pack(pop)

#pragma pack(push,1)
    struct AbonInfo : public InfoParser {
        uint8_t	stateActive;
        uint64_t IMSI;
        uint8_t	traceReference;
        uint16_t ARFCN;
        union {
                uint16_t chanId;
                uint16_t scramblingCode;
                uint16_t CRNTI;
        }  standartId;
        struct {
                uint8_t dymmy    : 5;
				uint8_t	standard : 3;
        }			bsType;
    };
#pragma pack(pop)
	enum Info {
		baseInfo = 0,
		abonInfo = 1
	};

	Info	info;
	System::DebugInfo&	debug;
	using Parse = std::function<void(const uint8_t* ptr, uint32_t bitSize)>;
        std::unordered_map<Info,Parse> parserMap;
    void ParseInfoMT(const uint8_t* ptr, uint32_t bitSize);
	void ParseInfoBS(const uint8_t* ptr, uint32_t bitSize);



        class BitReverser {
        public:
            BitReverser();
            ~BitReverser() {}

            enum Width {
                int8 =0,
                int16=1,
                int32=2,
                int64=3
            };

            void Do(const void* in, void* out, Width bit  );
        private:
            static const uint32_t tableSize = 256;

            std::array<uint8_t, tableSize> bitReverse;
            void Reverse8( const void* in, void* out );
            void Reverse16( const void* in, void* out );
            void Reverse32( const void* in, void* out );
            void Reverse64( const void* in, void* out );

            using Reverse = std::function<void(const void* in, void* out)>;
            std::unordered_map<Width,Reverse> reverseMap;
        };
        BitReverser reverser;

};

}
#endif
