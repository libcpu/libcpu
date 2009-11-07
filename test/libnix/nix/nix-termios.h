#ifndef __nix_termios_h
#define __nix_termios_h

/* Special Control Characters */
#define NIX_VEOF            0
#define NIX_VEOL            1
#define NIX_VEOL2           2
#define NIX_VERASE          3
#define NIX_VWERASE         4
#define NIX_VKILL           5
#define NIX_VREPRINT        6
#define NIX_VINTR           8
#define NIX_VQUIT           9
#define NIX_VSUSP           10
#define NIX_VDSUSP          11
#define NIX_VSTART          12
#define NIX_VSTOP           13
#define NIX_VLNEXT          14
#define NIX_VDISCARD        15
#define NIX_VMIN            16
#define NIX_VTIME           17
#define NIX_VSTATUS         18
#define NIX_NCCS            20

#define NIX__POSIX_VDISABLE 0xff
#define NIX_CCEQ(val, c)    ( (c) == (val) ? (val) != NIX__POSIX_VDISABLE : 0)

/* Input flags */
#define NIX_IGNBRK          0x00000001
#define NIX_BRKINT          0x00000002
#define NIX_IGNPAR          0x00000004
#define NIX_PARMRK          0x00000008
#define NIX_INPCK           0x00000010
#define NIX_ISTRIP          0x00000020
#define NIX_INLCR           0x00000040
#define NIX_IGNCR           0x00000080
#define NIX_ICRNL           0x00000100
#define NIX_IXON            0x00000200
#define NIX_IXOFF           0x00000400
#define NIX_IXANY           0x00000800
#define NIX_IUCLC           0x00001000
#define NIX_IMAXBEL         0x00002000

/* Output flags */
#define NIX_OPOST           0x00000001
#define NIX_ONLCR           0x00000002
#define NIX_OXTABS          0x00000004
#define NIX_ONOEOT          0x00000008
#define NIX_OCRNL           0x00000010
#define NIX_ONOCR           0x00000020
#define NIX_ONLRET          0x00000040
#define NIX_OFILL           0x00000080
#define NIX_NLDLY           0x00000300
#define NIX_TABDLY          0x00000c00
#define NIX_CRDLY           0x00003000
#define NIX_FFDLY           0x00004000
#define NIX_BSDLY           0x00008000
#define NIX_VTDLY           0x00010000
#define NIX_OFDEL           0x00020000
#define NIX_OLCUC           0x00040000

/* Control flags */
#define NIX_CIGNORE         0x00000001
#define NIX_CSIZE           0x00000300
#define   NIX_CS5           0x00000000
#define   NIX_CS6           0x00000100
#define   NIX_CS7           0x00000200
#define   NIX_CS8           0x00000300
#define NIX_CSTOPB          0x00000400
#define NIX_CREAD           0x00000800
#define NIX_PARENB          0x00001000
#define NIX_PARODD          0x00002000
#define NIX_HUPCL           0x00004000
#define NIX_CLOCAL          0x00008000
#define NIX_CCTS_OFLOW      0x00010000
#define NIX_CRTSCTS         (NIX_CCTS_OFLOW | NIX_CRTS_IFLOW)
#define NIX_CRTS_IFLOW      0x00020000
#define NIX_CDTR_IFLOW      0x00040000
#define NIX_CDSR_OFLOW      0x00080000
#define NIX_CCAR_OFLOW      0x00100000
#define NIX_MDMBUF          0x00200000

/* Local flags */
#define NIX_ECHOKE          0x00000001
#define NIX_ECHOE           0x00000002
#define NIX_ECHOK           0x00000004
#define NIX_ECHO            0x00000008
#define NIX_ECHONL          0x00000010
#define NIX_ECHOPRT         0x00000020
#define NIX_ECHOCTL         0x00000040
#define NIX_ISIG            0x00000080
#define NIX_ICANON          0x00000100
#define NIX_ALTWERASE       0x00000200
#define NIX_IEXTEN          0x00000400
#define NIX_EXTPROC         0x00000800
#define NIX_TOSTOP          0x00400000
#define NIX_FLUSHO          0x00800000
#define NIX_XCASE           0x01000000
#define NIX_NOKERNINFO      0x02000000
#define NIX_PENDIN          0x20000000
#define NIX_NOFLSH          0x80000000

/* Compat */
#define NIX_NL0     0x00000000
#define NIX_NL1     0x00000100
#define NIX_NL2     0x00000200
#define NIX_NL3     0x00000300
#define NIX_TAB0    0x00000000
#define NIX_TAB1    0x00000400
#define NIX_TAB2    0x00000800
#define NIX_TAB3    0x00000c00
#define NIX_CR0     0x00000000
#define NIX_CR1     0x00001000
#define NIX_CR2     0x00002000
#define NIX_CR3     0x00003000
#define NIX_FF0     0x00000000
#define NIX_FF1     0x00004000
#define NIX_BS0     0x00000000
#define NIX_BS1     0x00008000
#define NIX_VT0     0x00000000
#define NIX_VT1     0x00010000

typedef uint32_t nix_tcflag_t;
typedef uint32_t nix_speed_t;
typedef uint8_t  nix_cc_t;

struct nix_termios {
	nix_tcflag_t c_iflag;
	nix_tcflag_t c_oflag;
	nix_tcflag_t c_cflag;
	nix_tcflag_t c_lflag;
	nix_cc_t     c_cc[NIX_NCCS];
	nix_speed_t  c_ispeed;
	nix_speed_t  c_ospeed;
};

#endif  /* !__nix_termios_h */
