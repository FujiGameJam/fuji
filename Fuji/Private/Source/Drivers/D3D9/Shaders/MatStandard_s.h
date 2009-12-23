#if 0
//
// Generated by Microsoft (R) D3DX9 Shader Compiler 9.11.519.0000
//
//   fxc /FhMatStandard_s.h MatStandard.vsh /Tvs_1_1 /Zi /nologo
//
//
// Parameters:
//
//   float4 gColourMask;
//   float4 gModelColour;
//   float4 mTexMatrix[2];
//   float4x4 mWorldToScreen;
//
//
// Registers:
//
//   Name           Reg   Size
//   -------------- ----- ----
//   mWorldToScreen c0       4
//   mTexMatrix     c8       2
//   gModelColour   c10      1
//   gColourMask    c11      1
//

    vs_1_1
    dcl_position v0  // input<0,1,2,3>
    dcl_texcoord v1  // input<7,8,9,10>
    dcl_color v2  // input<11,12,13,14>

#line 87 "MatStandard.vsh"
    dp4 oPos.x, v0, c0  // output<0>
    dp4 oPos.y, v0, c1  // output<1>
    dp4 oPos.z, v0, c2  // output<2>

#line 96
    mul r0, v2, c10

#line 87
    dp4 oPos.w, v0, c3  // output<3>

#line 96
    mad oD0, r0, c11.xxxz, c11.yyyw  // output<4,5,6,7>

#line 90
    dp4 r0.x, v1, c8  // output<8>
    dp4 r0.y, v1, c9  // output<9>
    mov oT0, r0.xyyy  // ::main<8,9,10,11>
    mov oT1, r0.xyyy  // ::main<12,13,14,15>

// approximately 10 instruction slots used
#endif

const DWORD g_vs11_main_s[] =
{
    0xfffe0101, 0x0096fffe, 0x47554244, 0x00000028, 0x00000220, 0x00000000, 
    0x00000001, 0x00000038, 0x0000000d, 0x0000003c, 0x00000003, 0x000001e4, 
    0x000000a4, 0x5374614d, 0x646e6174, 0x2e647261, 0x00687376, 0x00000028, 
    0xffff0000, 0x00000374, 0xffff0000, 0x00000380, 0xffff0000, 0x0000038c, 
    0x00000057, 0x00000398, 0x00000057, 0x000003a8, 0x00000057, 0x000003b8, 
    0x00000060, 0x000003c8, 0x00000057, 0x000003d8, 0x00000060, 0x000003e8, 
    0x0000005a, 0x000003fc, 0x0000005b, 0x0000040c, 0x0000005a, 0x0000041c, 
    0x0000005a, 0x00000428, 0x6e69616d, 0x736f7000, 0xababab00, 0x00030001, 
    0x00040001, 0x00000001, 0x00000000, 0x6f6c6f63, 0x75007275, 0x76750076, 
    0xabab0032, 0x000000a9, 0x000000b0, 0x000000c0, 0x000000b0, 0x000000c7, 
    0x000000b0, 0x000000ca, 0x000000b0, 0x00000005, 0x00100001, 0x00040001, 
    0x000000d0, 0x0000000b, 0x00090008, 0x000b000a, 0x0000000c, 0x000d000c, 
    0x000f000e, 0x75706e69, 0x6f6e0074, 0xab006d72, 0x00030001, 0x00030001, 
    0x00000001, 0x00000000, 0x000000a9, 0x000000b0, 0x0000011e, 0x00000124, 
    0x000000c7, 0x000000b0, 0x000000c0, 0x000000b0, 0x00000005, 0x000f0001, 
    0x00040001, 0x00000134, 0x00000000, 0x00010000, 0x00030002, 0x00000001, 
    0x00080007, 0x000a0009, 0x00000002, 0x000c000b, 0x000e000d, 0x7074756f, 
    0xab007475, 0x00000003, 0xffff0000, 0xffffffff, 0x00000004, 0x0001ffff, 
    0xffffffff, 0x00000005, 0xffffffff, 0xffff0002, 0x00000007, 0xffffffff, 
    0x0003ffff, 0x00000008, 0x00050004, 0x00070006, 0x00000009, 0xffff0008, 
    0xffffffff, 0x0000000a, 0x0009ffff, 0xffffffff, 0x00000000, 0x000000a4, 
    0x000000f0, 0x00000002, 0x00000100, 0x000000a4, 0x00000118, 0x00000154, 
    0x00000003, 0x00000164, 0x000000a4, 0x00000188, 0x000000f0, 0x00000007, 
    0x00000190, 0x7263694d, 0x666f736f, 0x52282074, 0x33442029, 0x20395844, 
    0x64616853, 0x43207265, 0x69706d6f, 0x2072656c, 0x31312e39, 0x3931352e, 
    0x3030302e, 0xabab0030, 0x0044fffe, 0x42415443, 0x0000001c, 0x000000d7, 
    0xfffe0101, 0x00000004, 0x0000001c, 0x00000101, 0x000000d0, 0x0000006c, 
    0x000b0002, 0x00020001, 0x00000078, 0x00000000, 0x00000088, 0x000a0002, 
    0x00020001, 0x00000078, 0x00000000, 0x00000095, 0x00080002, 0x00020002, 
    0x000000a0, 0x00000000, 0x000000b0, 0x00000002, 0x00020004, 0x000000c0, 
    0x00000000, 0x6c6f4367, 0x4d72756f, 0x006b7361, 0x00030001, 0x00040001, 
    0x00000001, 0x00000000, 0x646f4d67, 0x6f436c65, 0x72756f6c, 0x65546d00, 
    0x74614d78, 0x00786972, 0x00030001, 0x00040001, 0x00000002, 0x00000000, 
    0x726f576d, 0x6f54646c, 0x65726353, 0xab006e65, 0x00030003, 0x00040004, 
    0x00000001, 0x00000000, 0x315f7376, 0x4d00315f, 0x6f726369, 0x74666f73, 
    0x29522820, 0x44334420, 0x53203958, 0x65646168, 0x6f432072, 0x6c69706d, 
    0x39207265, 0x2e31312e, 0x2e393135, 0x30303030, 0xababab00, 0x0000001f, 
    0x80000000, 0x900f0000, 0x0000001f, 0x80000005, 0x900f0001, 0x0000001f, 
    0x8000000a, 0x900f0002, 0x00000009, 0xc0010000, 0x90e40000, 0xa0e40000, 
    0x00000009, 0xc0020000, 0x90e40000, 0xa0e40001, 0x00000009, 0xc0040000, 
    0x90e40000, 0xa0e40002, 0x00000005, 0x800f0000, 0x90e40002, 0xa0e4000a, 
    0x00000009, 0xc0080000, 0x90e40000, 0xa0e40003, 0x00000004, 0xd00f0000, 
    0x80e40000, 0xa080000b, 0xa0d5000b, 0x00000009, 0x80010000, 0x90e40001, 
    0xa0e40008, 0x00000009, 0x80020000, 0x90e40001, 0xa0e40009, 0x00000001, 
    0xe00f0000, 0x80540000, 0x00000001, 0xe00f0001, 0x80540000, 0x0000ffff
};
