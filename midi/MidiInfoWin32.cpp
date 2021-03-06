#include <Windows.h>
#include <Mmsystem.h>
#include <Mmreg.h>
#include <vector>
#include <string>
#include <sstream>

std::vector<std::wstring> GetInfo(void*p)
{
    std::vector<std::wstring> v;
    MIDIOUTCAPS2* cap = (MIDIOUTCAPS2*)p;
    v.push_back(cap->szPname);

    std::wstring man;
    std::wstring kname=L"SYSTEM\\CurrentControlSet\\Control\\MediaCategories\\";
    OLECHAR* olestr;
    StringFromCLSID(cap->ManufacturerGuid, &olestr);
    kname+=olestr;
    ::CoTaskMemFree(olestr);
    HKEY hKey;
    if (ERROR_SUCCESS==RegOpenKeyExW(HKEY_LOCAL_MACHINE, kname.c_str(), 0, KEY_READ, &hKey)) {
        WCHAR szBuffer[512];
        DWORD dwBufferSize = sizeof(szBuffer);
        if (ERROR_SUCCESS==RegQueryValueExW(hKey, L"Name", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize)) man=szBuffer;
        RegCloseKey(hKey);
    }
    if (man.empty()) {
        switch(cap->wMid) {
            case MM_MICROSOFT:	man=L"Microsoft Corporation"; break;
            case MM_CREATIVE:	man=L"Creative Labs, Inc."; break;
            case MM_MEDIAVISION:	man=L"Media Vision, Inc."; break;
            case MM_FUJITSU:	man=L"Fujitsu Corp."; break;
            case MM_PRAGMATRAX:	man=L"PRAGMATRAX Software"; break;
            case MM_CYRIX:		man=L"Cyrix Corporation"; break;
            case MM_PHILIPS_SPEECH_PROCESSING:	man=L"Philips Speech Processing"; break;
            case MM_NETXL:		man=L"NetXL, Inc."; break;
            case MM_ZYXEL:		man=L"ZyXEL Communications, Inc."; break;
            case MM_BECUBED:	man=L"BeCubed Software Inc."; break;
            case MM_AARDVARK:	man=L"Aardvark Computer Systems, Inc."; break;
            case MM_BINTEC:		man=L"Bin Tec Communications GmbH"; break;
            case MM_HEWLETT_PACKARD:	man=L"Hewlett-Packard Company"; break;
            case MM_ACULAB:		man=L"Aculab plc"; break;
            case MM_FAITH:		man=L"Faith,Inc."; break;
            case MM_MITEL:		man=L"Mitel Corporation"; break;
            case MM_QUANTUM3D:	man=L"Quantum3D, Inc."; break;
            case MM_SNI:		man=L"Siemens-Nixdorf"; break;
            case MM_EMU:		man=L"E-mu Systems, Inc."; break;
            case MM_ARTISOFT:	man=L"Artisoft, Inc."; break;
            case MM_TURTLE_BEACH:	man=L"Turtle Beach, Inc."; break;
            case MM_IBM:		man=L"IBM Corporation"; break;
            case MM_VOCALTEC:	man=L"Vocaltec Ltd."; break;
            case MM_ROLAND:		man=L"Roland"; break;
            case MM_DSP_SOLUTIONS:	man=L"DSP Solutions, Inc."; break;
            case MM_NEC:		man=L"NEC"; break;
            case MM_ATI:		man=L"ATI Technologies Inc."; break;
            case MM_WANGLABS:	man=L"Wang Laboratories, Inc."; break;
            case MM_TANDY:		man=L"Tandy Corporation"; break;
            case MM_VOYETRA:	man=L"Voyetra"; break;
            case MM_ANTEX:		man=L"Antex Electronics Corporation"; break;
            case MM_ICL_PS:		man=L"ICL Personal Systems"; break;
            case MM_INTEL:		man=L"Intel Corporation"; break;
            case MM_GRAVIS:		man=L"Advanced Gravis"; break;
            case MM_VAL:		man=L"Video Associates Labs, Inc."; break;
            case MM_INTERACTIVE:	man=L"InterActive Inc."; break;
            case MM_YAMAHA:		man=L"Yamaha Corporation"; break;
            case MM_EVEREX:		man=L"Everex Systems, Inc."; break;
            case MM_ECHO:		man=L"Echo Speech Corporation"; break;
            case MM_SIERRA:		man=L"Sierra Semiconductor Corp"; break;
            case MM_CAT:		man=L"Computer Aided Technologies"; break;
            case MM_APPS:		man=L"APPS Software International"; break;
            case MM_DSP_GROUP:	man=L"DSP Group, Inc."; break;
            case MM_MELABS:		man=L"microEngineering Labs"; break;
            case MM_COMPUTER_FRIENDS:	man=L"Computer Friends, Inc."; break;
            case MM_ESS:		man=L"ESS Technology"; break;
            case MM_AUDIOFILE:	man=L"Audio, Inc."; break;
            case MM_MOTOROLA:	man=L"Motorola, Inc."; break;
            case MM_CANOPUS:	man=L"Canopus, co., Ltd."; break;
            case MM_EPSON:		man=L"Seiko Epson Corporation"; break;
            case MM_TRUEVISION:	man=L"Truevision"; break;
            case MM_AZTECH:		man=L"Aztech Labs, Inc."; break;
            case MM_VIDEOLOGIC:	man=L"Videologic"; break;
            case MM_SCALACS:	man=L"SCALACS"; break;
            case MM_KORG:		man=L"Korg Inc."; break;
            case MM_APT:		man=L"Audio Processing Technology"; break;
            case MM_ICS:		man=L"Integrated Circuit Systems, Inc."; break;
            case MM_ITERATEDSYS:	man=L"Iterated Systems, Inc."; break;
            case MM_METHEUS:	man=L"Metheus"; break;
            case MM_LOGITECH:	man=L"Logitech, Inc."; break;
            case MM_WINNOV:		man=L"Winnov, Inc."; break;
            case MM_NCR:		man=L"NCR Corporation"; break;
            case MM_EXAN:		man=L"EXAN"; break;
            case MM_AST:		man=L"AST Research Inc."; break;
            case MM_WILLOWPOND:	man=L"Willow Pond Corporation"; break;
            case MM_SONICFOUNDRY:	man=L"Sonic Foundry"; break;
            case MM_VITEC:		man=L"Vitec Multimedia"; break;
            case MM_MOSCOM:		man=L"MOSCOM Corporation"; break;
            case MM_SILICONSOFT:	man=L"Silicon Soft, Inc."; break;
            case MM_TERRATEC:	man=L"TerraTec Electronic GmbH"; break;
            case MM_MEDIASONIC:	man=L"MediaSonic Ltd."; break;
            case MM_SANYO:		man=L"SANYO Electric Co., Ltd."; break;
            case MM_SUPERMAC:	man=L"Supermac"; break;
            case MM_AUDIOPT:	man=L"Audio Processing Technology"; break;
            case MM_NOGATECH:	man=L"NOGATECH Ltd."; break;
            case MM_SPEECHCOMP:	man=L"Speech Compression"; break;
            case MM_AHEAD:		man=L"Ahead, Inc."; break;
            case MM_DOLBY:		man=L"Dolby Laboratories"; break;
            case MM_OKI:		man=L"OKI"; break;
            case MM_AURAVISION:	man=L"AuraVision Corporation"; break;
            case MM_OLIVETTI:	man=L"Ing C. Olivetti & C., S.p.A."; break;
            case MM_IOMAGIC:	man=L"I/O Magic Corporation"; break;
            case MM_MATSUSHITA:	man=L"Matsushita Electric Industrial Co., Ltd."; break;
            case MM_CONTROLRES:	man=L"Control Resources Limited"; break;
            case MM_XEBEC:		man=L"Xebec Multimedia Solutions Limited"; break;
            case MM_NEWMEDIA:	man=L"New Media Corporation"; break;
            case MM_NMS:		man=L"Natural MicroSystems"; break;
            case MM_LYRRUS:		man=L"Lyrrus Inc."; break;
            case MM_COMPUSIC:	man=L"Compusic"; break;
            case MM_OPTI:		man=L"OPTi Computers Inc."; break;
            case MM_ADLACC:		man=L"Adlib Accessories Inc."; break;
            case MM_COMPAQ:		man=L"Compaq Computer Corp."; break;
            case MM_DIALOGIC:	man=L"Dialogic Corporation"; break;
            case MM_INSOFT:		man=L"InSoft, Inc."; break;
            case MM_MPTUS:		man=L"M.P. Technologies, Inc."; break;
            case MM_WEITEK:		man=L"Weitek"; break;
            case MM_LERNOUT_AND_HAUSPIE:	man=L"Lernout & Hauspie"; break;
            case MM_QCIAR:		man=L"Quanta Computer Inc."; break;
            case MM_APPLE:		man=L"Apple Computer, Inc."; break;
            case MM_DIGITAL:	man=L"Digital Equipment Corporation"; break;
            case MM_MOTU:		man=L"Mark of the Unicorn"; break;
            case MM_WORKBIT:	man=L"Workbit Corporation"; break;
            case MM_OSITECH:	man=L"Ositech Communications Inc."; break;
            case MM_MIRO:		man=L"miro Computer Products AG"; break;
            case MM_CIRRUSLOGIC:	man=L"Cirrus Logic"; break;
            case MM_ISOLUTION:	man=L"ISOLUTION  B.V."; break;
            case MM_HORIZONS:	man=L"Horizons Technology, Inc."; break;
            case MM_CONCEPTS:	man=L"Computer Concepts Ltd."; break;
            case MM_VTG:		man=L"Voice Technologies Group, Inc."; break;
            case MM_RADIUS:		man=L"Radius"; break;
            case MM_ROCKWELL:	man=L"Rockwell International"; break;
            case MM_XYZ:		man=L"Co. XYZ for testing"; break;
            case MM_OPCODE:		man=L"Opcode Systems"; break;
            case MM_VOXWARE:	man=L"Voxware Inc."; break;
            case MM_NORTHERN_TELECOM:	man=L"Northern Telecom Limited"; break;
            case MM_APICOM:		man=L"APICOM"; break;
            case MM_GRANDE:		man=L"Grande Software"; break;
            case MM_ADDX:		man=L"ADDX"; break;
            case MM_WILDCAT:	man=L"Wildcat Canyon Software"; break;
            case MM_RHETOREX:	man=L"Rhetorex Inc."; break;
            case MM_BROOKTREE:	man=L"Brooktree Corporation"; break;
            case MM_ENSONIQ:	man=L"ENSONIQ Corporation"; break;
            case MM_FAST:		man=L"FAST Multimedia AG"; break;
            case MM_NVIDIA:		man=L"NVidia Corporation"; break;
            case MM_OKSORI:		man=L"OKSORI Co., Ltd."; break;
            case MM_DIACOUSTICS:	man=L"DiAcoustics, Inc."; break;
            case MM_GULBRANSEN:	man=L"Gulbransen, Inc."; break;
            case MM_KAY_ELEMETRICS:	man=L"Kay Elemetrics, Inc."; break;
            case MM_CRYSTAL:	man=L"Crystal Semiconductor Corporation"; break;
            case MM_SPLASH_STUDIOS:	man=L"Splash Studios"; break;
            case MM_QUARTERDECK:	man=L"Quarterdeck Corporation"; break;
            case MM_TDK:		man=L"TDK Corporation"; break;
            case MM_DIGITAL_AUDIO_LABS:	man=L"Digital Audio Labs, Inc."; break;
            case MM_SEERSYS:	man=L"Seer Systems, Inc."; break;
            case MM_PICTURETEL:	man=L"PictureTel Corporation"; break;
            case MM_ATT_MICROELECTRONICS:	man=L"AT&T Microelectronics"; break;
            case MM_OSPREY:		man=L"Osprey Technologies, Inc."; break;
            case MM_MEDIATRIX:	man=L"Mediatrix Peripherals"; break;
            case MM_SOUNDESIGNS:	man=L"SounDesignS M.C.S. Ltd."; break;
            case MM_ALDIGITAL:	man=L"A.L. Digital Ltd."; break;
            case MM_SPECTRUM_SIGNAL_PROCESSING:	man=L"Spectrum Signal Processing, Inc."; break;
            case MM_ECS:		man=L"Electronic Courseware Systems, Inc."; break;
            case MM_AMD:		man=L"AMD"; break;
            case MM_COREDYNAMICS:	man=L"Core Dynamics"; break;
            case MM_CANAM:		man=L"CANAM Computers"; break;
            case MM_SOFTSOUND:	man=L"Softsound, Ltd."; break;
            case MM_NORRIS:		man=L"Norris Communications, Inc."; break;
            case MM_DDD:		man=L"Danka Data Devices"; break;
            case MM_EUPHONICS:	man=L"EuPhonics"; break;
            case MM_PRECEPT:	man=L"Precept Software, Inc."; break;
            case MM_CRYSTAL_NET:	man=L"Crystal Net Corporation"; break;
            case MM_CHROMATIC:	man=L"Chromatic Research, Inc."; break;
            case MM_VOICEINFO:	man=L"Voice Information Systems, Inc."; break;
            case MM_VIENNASYS:	man=L"Vienna Systems"; break;
            case MM_CONNECTIX:	man=L"Connectix Corporation"; break;
            case MM_GADGETLABS:	man=L"Gadget Labs LLC"; break;
            case MM_FRONTIER:	man=L"Frontier Design Group LLC"; break;
            case MM_VIONA:		man=L"Viona Development GmbH"; break;
            case MM_CASIO:		man=L"Casio Computer Co., LTD"; break;
            case MM_DIAMONDMM:	man=L"Diamond Multimedia"; break;
            case MM_S3:			man=L"S3"; break;
            case MM_DVISION:	man=L"D-Vision Systems, Inc."; break;
            case MM_NETSCAPE:	man=L"Netscape Communications"; break;
            case MM_SOUNDSPACE:	man=L"Soundspace Audio"; break;
            case MM_VANKOEVERING:	man=L"VanKoevering Company"; break;
            case MM_QTEAM:		man=L"Q-Team"; break;
            case MM_ZEFIRO:		man=L"Zefiro Acoustics"; break;
            case MM_STUDER:		man=L"Studer Professional Audio AG"; break;
            case MM_FRAUNHOFER_IIS:	man=L"Fraunhofer IIS"; break;
            case MM_QUICKNET:	man=L"Quicknet Technologies"; break;
            case MM_ALARIS:		man=L"Alaris, Inc."; break;
            case MM_SICRESOURCE:	man=L"SIC Resource Inc."; break;
            case MM_NEOMAGIC:	man=L"NeoMagic Corporation"; break;
            case MM_MERGING_TECHNOLOGIES:	man=L"Merging Technologies S.A."; break;
            case MM_XIRLINK:	man=L"Xirlink, Inc."; break;
            case MM_COLORGRAPH:	man=L"Colorgraph Ltd"; break;
            case MM_OTI:		man=L"Oak Technology, Inc."; break;
            case MM_AUREAL:		man=L"Aureal Semiconductor"; break;
            case MM_VIVO:		man=L"Vivo Software"; break;
            case MM_SHARP:		man=L"Sharp"; break;
            case MM_LUCENT:		man=L"Lucent Technologies"; break;
            case MM_ATT:		man=L"AT&T Labs, Inc."; break;
            case MM_SUNCOM:		man=L"Sun Communications, Inc."; break;
            case MM_SORVIS:		man=L"Sorenson Vision"; break;
            case MM_INVISION:	man=L"InVision Interactive"; break;
            case MM_BERKOM:		man=L"Deutsche Telekom Berkom GmbH"; break;
            case MM_MARIAN:		man=L"Marian GbR Leipzig"; break;
            case MM_DPSINC:		man=L"Digital Processing Systems, Inc."; break;
            case MM_BCB:		man=L"BCB Holdings Inc."; break;
            case MM_MOTIONPIXELS:	man=L"Motion Pixels"; break;
            case MM_QDESIGN:	man=L"QDesign Corporation"; break;
            case MM_NMP:		man=L"Nokia Mobile Phones"; break;
            case MM_DATAFUSION:	man=L"DataFusion Systems"; break;
            case MM_DUCK:		man=L"The Duck Corporation"; break;
            case MM_FTR:		man=L"Future Technology Resources"; break;
            case MM_BERCOS:		man=L"BERCOS GmbH"; break;
            case MM_ONLIVE:		man=L"OnLive! Technologies, Inc."; break;
            case MM_SIEMENS_SBC:	man=L"Siemens Business Communications Systems"; break;
            case MM_TERALOGIC:	man=L"TeraLogic, Inc."; break;
            case MM_PHONET:		man=L"PhoNet Communications Ltd."; break;
            case MM_WINBOND:	man=L"Winbond Electronics Corp"; break;
            case MM_VIRTUALMUSIC:	man=L"Virtual Music, Inc."; break;
            case MM_ENET:		man=L"e-Net, Inc."; break;
            case MM_GUILLEMOT:	man=L"Guillemot International"; break;
            case MM_EMAGIC:		man=L"Emagic Soft- und Hardware GmbH"; break;
            case MM_MWM:		man=L"MWM Acoustics LLC"; break;
            case MM_PACIFICRESEARCH:	man=L"Pacific Research and Engineering Corporation"; break;
            case MM_SIPROLAB:	man=L"Sipro Lab Telecom Inc."; break;
            case MM_LYNX:		man=L"Lynx Studio Technology, Inc."; break;
            case MM_SPECTRUM_PRODUCTIONS:	man=L"Spectrum Productions"; break;
            case MM_DICTAPHONE:	man=L"Dictaphone Corporation"; break;
            case MM_QUALCOMM:	man=L"QUALCOMM, Inc."; break;
            case MM_RZS:		man=L"Ring Zero Systems, Inc"; break;
            case MM_AUDIOSCIENCE:	man=L"AudioScience Inc."; break;
            case MM_PINNACLE:	man=L"Pinnacle Systems, Inc."; break;
            case MM_EES:		man=L"EES Technik fuer Musik GmbH"; break;
            case MM_HAFTMANN:	man=L"haftmann#software"; break;
            case MM_LUCID:		man=L"Lucid Technology, Symetrix Inc."; break;
            case MM_HEADSPACE:	man=L"Headspace, Inc"; break;
            case MM_UNISYS:		man=L"UNISYS CORPORATION"; break;
            case MM_LUMINOSITI:	man=L"Luminositi, Inc."; break;
            case MM_ACTIVEVOICE:	man=L"ACTIVE VOICE CORPORATION"; break;
            case MM_DTS:		man=L"Digital Theater Systems, Inc."; break;
            case MM_DIGIGRAM:	man=L"DIGIGRAM"; break;
            case MM_SOFTLAB_NSK:	man=L"Softlab-Nsk"; break;
            case MM_FORTEMEDIA:	man=L"ForteMedia, Inc"; break;
            case MM_SONORUS:	man=L"Sonorus, Inc."; break;
            case MM_ARRAY:		man=L"Array Microsystems, Inc."; break;
            case MM_DATARAN:	man=L"Data Translation, Inc."; break;
            case MM_I_LINK:		man=L"I-link Worldwide"; break;
            case MM_SELSIUS_SYSTEMS:	man=L"Selsius Systems Inc."; break;
            case MM_ADMOS:		man=L"AdMOS Technology, Inc."; break;
            case MM_LEXICON:	man=L"Lexicon Inc."; break;
            case MM_SGI:		man=L"Silicon Graphics Inc."; break;
            case MM_IPI:		man=L"Interactive Product Inc."; break;
            case MM_ICE:		man=L"IC Ensemble, Inc."; break;
            case MM_VQST:		man=L"ViewQuest Technologies Inc."; break;
            case MM_ETEK:		man=L"eTEK Labs Inc."; break;
            case MM_CS:			man=L"Consistent Software"; break;
            case MM_ALESIS:		man=L"Alesis Studio Electronics"; break;
            case MM_INTERNET:	man=L"INTERNET Corporation"; break;
            case MM_SONY:		man=L"Sony Corporation"; break;
            case MM_HYPERACTIVE:	man=L"Hyperactive Audio Systems, Inc."; break;
            case MM_UHER_INFORMATIC:	man=L"UHER informatic GmbH"; break;
            case MM_SYDEC_NV:	man=L"Sydec NV"; break;
            case MM_FLEXION:	man=L"Flexion Systems Ltd."; break;
            case MM_VIA:		man=L"Via Technologies, Inc."; break;
            case MM_MICRONAS:	man=L"Micronas Semiconductors, Inc."; break;
            case MM_ANALOGDEVICES:	man=L"Analog Devices, Inc."; break;
            case MM_HP:			man=L"Hewlett Packard Company"; break;
            case MM_MATROX_DIV:	man=L"Matrox"; break;
            case MM_QUICKAUDIO:	man=L"Quick Audio, GbR"; break;
            case MM_YOUCOM:		man=L"You/Com Audiocommunicatie BV"; break;
            case MM_RICHMOND:	man=L"Richmond Sound Design Ltd."; break;
            case MM_IODD:		man=L"I-O Data Device, Inc."; break;
            case MM_ICCC:		man=L"ICCC A/S"; break;
            case MM_3COM:		man=L"3COM Corporation"; break;
            case MM_MALDEN:		man=L"Malden Electronics Ltd."; break;
            case MM_3DFX:		man=L"3Dfx Interactive, Inc."; break;
            case MM_MINDMAKER:	man=L"Mindmaker, Inc."; break;
            case MM_TELEKOL:	man=L"Telekol Corp."; break;
            case MM_ST_MICROELECTRONICS:	man=L"ST Microelectronics"; break;
            case MM_ALGOVISION:	man=L"Algo Vision Systems GmbH"; break;
            default: man=L"unknown";
        }
    }
    v.push_back(man);
    
    std::wostringstream ver;
    ver<<(cap->vDriverVersion>>8)<<'.'<<(cap->vDriverVersion&0xff);
    v.push_back(ver.str());
    return v;
}
