#ifndef KIOSK_ADDRS_H
#define KIOSK_ADDRS_H

class Addrs {
public:
    const struct Home {
        static const u16 Touch = 0x1000;
    } h;

    static const struct Admin {
        const struct Main {
            static const u16 InitRandom = 0x1100;
            static const u16 InitAdmin = 0x1101;
            static const u16 AddBonusPoint = 0x1102;
            static const u16 Exit = 0x1103;

            static const u16 Strength = 0x1000;
            static const u16 Dexterity = 0x1001;
            static const u16 Magic = 0x1002;
            static const u16 Name = 0x1005;

            static const u16 IncStrength = 0x1010;
            static const u16 IncDexterity = 0x1020;
            static const u16 IncMagic = 0x1030;
            static const u16 DecStrength = 0x1011;
            static const u16 DecDexterity = 0x1021;
            static const u16 DecMagic = 0x1031;
        } m;
        const struct Error {
            static const u16 Text = 0X1000;
        } e;
    } a;

    static const struct User {
        const struct Main {
            static const u16 Strength = 0x1000;
            static const u16 Dexterity = 0x1001;
            static const u16 Magic = 0x1002;
            static const u16 Name = 0x1005;

            static const u16 SkillsButton = 0x1010;
        } m;

        const struct BonusPoints {
            static const u16 Strength = 0x1000;
            static const u16 Dexterity = 0x1001;
            static const u16 Magic = 0x1002;
            static const u16 Name = 0x1005;

            static const u16 IncStrength = 0x1100;
            static const u16 IncDexterity = 0x1101;
            static const u16 IncMagic = 0x1102;
        } bp;

        const struct Skills {
            static const u16 Name = 0x1005;

            static const u16 BackButton = 0x1010;
            static const u16 PrevButton = 0x1011;
            static const u16 NextButton = 0x1012;

            static const u16 VpAddrBase = 0x1100;
            static const u16 SpAddrBase = 0x2100;
            // other skill addrs for texts are derived: Base + row * 0x0100 + col * 0x0030
        } s;
    } u;
};

#endif //KIOSK_ADDRS_H
