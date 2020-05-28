unsigned char float_cl[] = {
  0x0d, 0x0a, 0x5f, 0x5f, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x20, 0x76,
  0x6f, 0x69, 0x64, 0x20, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x65, 0x28,
  0x5f, 0x5f, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x2a, 0x20, 0x41, 0x2c, 0x20, 0x63, 0x6f, 0x6e, 0x73, 0x74,
  0x20, 0x69, 0x6e, 0x74, 0x20, 0x77, 0x69, 0x64, 0x74, 0x68, 0x2c, 0x20,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x78, 0x6c, 0x2c, 0x20, 0x66, 0x6c,
  0x6f, 0x61, 0x74, 0x20, 0x79, 0x74, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61,
  0x74, 0x20, 0x70, 0x69, 0x78, 0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65,
  0x58, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x70, 0x69, 0x78,
  0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x59, 0x2c, 0x20, 0x69, 0x6e,
  0x74, 0x20, 0x6d, 0x61, 0x78, 0x2c, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x73,
  0x6d, 0x6f, 0x6f, 0x74, 0x68, 0x2c, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x6a,
  0x75, 0x6c, 0x69, 0x61, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20,
  0x6a, 0x75, 0x6c, 0x69, 0x61, 0x58, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61,
  0x74, 0x20, 0x6a, 0x75, 0x6c, 0x69, 0x61, 0x59, 0x29, 0x20, 0x7b, 0x0d,
  0x0a, 0x20, 0x20, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x69, 0x6e, 0x64, 0x65,
  0x78, 0x20, 0x3d, 0x20, 0x67, 0x65, 0x74, 0x5f, 0x67, 0x6c, 0x6f, 0x62,
  0x61, 0x6c, 0x5f, 0x69, 0x64, 0x28, 0x30, 0x29, 0x3b, 0x0d, 0x0a, 0x20,
  0x20, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x78, 0x20, 0x3d, 0x20, 0x69, 0x6e,
  0x64, 0x65, 0x78, 0x20, 0x25, 0x20, 0x77, 0x69, 0x64, 0x74, 0x68, 0x3b,
  0x0d, 0x0a, 0x20, 0x20, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x79, 0x20, 0x3d,
  0x20, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x20, 0x2f, 0x20, 0x77, 0x69, 0x64,
  0x74, 0x68, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61,
  0x74, 0x20, 0x61, 0x20, 0x3d, 0x20, 0x78, 0x20, 0x2a, 0x20, 0x70, 0x69,
  0x78, 0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x58, 0x20, 0x2b, 0x20,
  0x78, 0x6c, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61,
  0x74, 0x20, 0x62, 0x20, 0x3d, 0x20, 0x79, 0x20, 0x2a, 0x20, 0x70, 0x69,
  0x78, 0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x59, 0x20, 0x2b, 0x20,
  0x79, 0x74, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61,
  0x74, 0x20, 0x63, 0x61, 0x20, 0x3d, 0x20, 0x6a, 0x75, 0x6c, 0x69, 0x61,
  0x20, 0x21, 0x3d, 0x20, 0x30, 0x20, 0x3f, 0x20, 0x6a, 0x75, 0x6c, 0x69,
  0x61, 0x58, 0x20, 0x3a, 0x20, 0x61, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x63, 0x62, 0x20, 0x3d, 0x20, 0x6a,
  0x75, 0x6c, 0x69, 0x61, 0x20, 0x21, 0x3d, 0x20, 0x30, 0x20, 0x3f, 0x20,
  0x6a, 0x75, 0x6c, 0x69, 0x61, 0x59, 0x20, 0x3a, 0x20, 0x62, 0x3b, 0x0d,
  0x0a, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x6e, 0x20,
  0x3d, 0x20, 0x30, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x77, 0x68, 0x69,
  0x6c, 0x65, 0x20, 0x28, 0x6e, 0x20, 0x3c, 0x20, 0x6d, 0x61, 0x78, 0x20,
  0x2d, 0x20, 0x31, 0x29, 0x20, 0x7b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x61, 0x61, 0x20,
  0x3d, 0x20, 0x61, 0x20, 0x2a, 0x20, 0x61, 0x3b, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x62,
  0x62, 0x20, 0x3d, 0x20, 0x62, 0x20, 0x2a, 0x20, 0x62, 0x3b, 0x0d, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74,
  0x20, 0x61, 0x62, 0x20, 0x3d, 0x20, 0x61, 0x20, 0x2a, 0x20, 0x62, 0x3b,
  0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x61, 0x20, 0x3d,
  0x20, 0x61, 0x61, 0x20, 0x2d, 0x20, 0x62, 0x62, 0x20, 0x2b, 0x20, 0x63,
  0x61, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x62,
  0x20, 0x3d, 0x20, 0x61, 0x62, 0x20, 0x2b, 0x20, 0x61, 0x62, 0x20, 0x2b,
  0x20, 0x63, 0x62, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x69, 0x66, 0x20, 0x28, 0x61, 0x61, 0x20, 0x2b, 0x20, 0x62, 0x62,
  0x20, 0x3e, 0x20, 0x31, 0x36, 0x29, 0x20, 0x62, 0x72, 0x65, 0x61, 0x6b,
  0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x6e, 0x2b,
  0x2b, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x7d, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x69, 0x66, 0x20, 0x28, 0x6e, 0x20, 0x3e, 0x3d, 0x20, 0x6d, 0x61,
  0x78, 0x20, 0x2d, 0x20, 0x31, 0x29, 0x20, 0x7b, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x41, 0x5b, 0x69, 0x6e, 0x64, 0x65, 0x78,
  0x5d, 0x20, 0x3d, 0x20, 0x6d, 0x61, 0x78, 0x3b, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x7d, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x65, 0x6c, 0x73, 0x65, 0x20,
  0x7b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x69, 0x66,
  0x20, 0x28, 0x73, 0x6d, 0x6f, 0x6f, 0x74, 0x68, 0x20, 0x21, 0x3d, 0x20,
  0x30, 0x29, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x41, 0x5b, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x5d, 0x20,
  0x3d, 0x20, 0x28, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x29, 0x6e, 0x29,
  0x20, 0x2b, 0x20, 0x31, 0x20, 0x2d, 0x20, 0x6c, 0x6f, 0x67, 0x28, 0x6c,
  0x6f, 0x67, 0x28, 0x61, 0x20, 0x2a, 0x20, 0x61, 0x20, 0x2b, 0x20, 0x62,
  0x20, 0x2a, 0x20, 0x62, 0x29, 0x20, 0x2f, 0x20, 0x32, 0x29, 0x20, 0x2f,
  0x20, 0x6c, 0x6f, 0x67, 0x28, 0x32, 0x2e, 0x30, 0x66, 0x29, 0x3b, 0x0d,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x65, 0x6c, 0x73, 0x65,
  0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x41, 0x5b, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x5d, 0x20, 0x3d, 0x20,
  0x28, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x29, 0x6e, 0x29, 0x3b, 0x0d,
  0x0a, 0x20, 0x20, 0x20, 0x7d, 0x0d, 0x0a, 0x7d, 0x0d, 0x0a, 0x0d, 0x0a,
  0x0d, 0x0a, 0x2f, 0x2a, 0x0d, 0x0a, 0x5f, 0x5f, 0x6b, 0x65, 0x72, 0x6e,
  0x65, 0x6c, 0x20, 0x76, 0x6f, 0x69, 0x64, 0x20, 0x69, 0x74, 0x65, 0x72,
  0x61, 0x74, 0x65, 0x5f, 0x76, 0x65, 0x63, 0x34, 0x28, 0x5f, 0x5f, 0x67,
  0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x2a,
  0x20, 0x41, 0x2c, 0x20, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x20, 0x69, 0x6e,
  0x74, 0x20, 0x77, 0x69, 0x64, 0x74, 0x68, 0x2c, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x20, 0x78, 0x6c, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74,
  0x20, 0x79, 0x74, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x70,
  0x69, 0x78, 0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x58, 0x2c, 0x20,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x70, 0x69, 0x78, 0x65, 0x6c, 0x53,
  0x63, 0x61, 0x6c, 0x65, 0x59, 0x2c, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x6d,
  0x61, 0x78, 0x2c, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x73, 0x6d, 0x6f, 0x6f,
  0x74, 0x68, 0x2c, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x6a, 0x75, 0x6c, 0x69,
  0x61, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x6a, 0x75, 0x6c,
  0x69, 0x61, 0x58, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20, 0x6a,
  0x75, 0x6c, 0x69, 0x61, 0x59, 0x29, 0x20, 0x7b, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x69, 0x6e, 0x74, 0x20, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x20, 0x3d,
  0x20, 0x67, 0x65, 0x74, 0x5f, 0x67, 0x6c, 0x6f, 0x62, 0x61, 0x6c, 0x5f,
  0x69, 0x64, 0x28, 0x30, 0x29, 0x20, 0x2a, 0x20, 0x34, 0x3b, 0x0d, 0x0a,
  0x20, 0x20, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x78, 0x20, 0x3d, 0x20, 0x69,
  0x6e, 0x64, 0x65, 0x78, 0x20, 0x25, 0x20, 0x77, 0x69, 0x64, 0x74, 0x68,
  0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x69, 0x6e, 0x74, 0x20, 0x79, 0x20,
  0x3d, 0x20, 0x69, 0x6e, 0x64, 0x65, 0x78, 0x20, 0x2f, 0x20, 0x77, 0x69,
  0x64, 0x74, 0x68, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x20, 0x61, 0x20, 0x3d, 0x20, 0x28, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x29, 0x20, 0x28, 0x78, 0x20, 0x2a, 0x20, 0x70, 0x69,
  0x78, 0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x58, 0x20, 0x2b, 0x20,
  0x78, 0x6c, 0x2c, 0x20, 0x28, 0x78, 0x20, 0x2b, 0x20, 0x31, 0x29, 0x20,
  0x2a, 0x20, 0x70, 0x69, 0x78, 0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65,
  0x58, 0x20, 0x2b, 0x20, 0x78, 0x6c, 0x2c, 0x20, 0x28, 0x78, 0x20, 0x2b,
  0x20, 0x32, 0x29, 0x20, 0x2a, 0x20, 0x70, 0x69, 0x78, 0x65, 0x6c, 0x53,
  0x63, 0x61, 0x6c, 0x65, 0x58, 0x20, 0x2b, 0x20, 0x78, 0x6c, 0x2c, 0x20,
  0x28, 0x78, 0x20, 0x2b, 0x20, 0x33, 0x29, 0x20, 0x2a, 0x20, 0x70, 0x69,
  0x78, 0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x58, 0x20, 0x2b, 0x20,
  0x78, 0x6c, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x20, 0x62, 0x20, 0x3d, 0x20, 0x28, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x29, 0x20, 0x28, 0x79, 0x20, 0x2a, 0x20, 0x70, 0x69,
  0x78, 0x65, 0x6c, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x59, 0x20, 0x2b, 0x20,
  0x79, 0x74, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x20, 0x63, 0x61, 0x20, 0x3d, 0x20, 0x6a, 0x75, 0x6c,
  0x69, 0x61, 0x20, 0x3f, 0x20, 0x28, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74,
  0x34, 0x29, 0x28, 0x6a, 0x75, 0x6c, 0x69, 0x61, 0x58, 0x29, 0x29, 0x20,
  0x3a, 0x20, 0x61, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x20, 0x63, 0x62, 0x20, 0x3d, 0x20, 0x6a, 0x75, 0x6c,
  0x69, 0x61, 0x20, 0x3f, 0x20, 0x28, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74,
  0x34, 0x29, 0x28, 0x6a, 0x75, 0x6c, 0x69, 0x61, 0x59, 0x29, 0x29, 0x20,
  0x3a, 0x20, 0x62, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x20, 0x72, 0x65, 0x73, 0x61, 0x20, 0x3d, 0x20, 0x28,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x29, 0x28, 0x30, 0x29, 0x3b, 0x0d,
  0x0a, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x20, 0x72,
  0x65, 0x73, 0x62, 0x20, 0x3d, 0x20, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74,
  0x34, 0x29, 0x28, 0x30, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x66,
  0x6c, 0x6f, 0x61, 0x74, 0x34, 0x20, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x20,
  0x3d, 0x20, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x29, 0x28, 0x30,
  0x29, 0x3b, 0x0d, 0x0a, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x69, 0x6e, 0x74,
  0x20, 0x6e, 0x20, 0x3d, 0x20, 0x30, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20,
  0x69, 0x66, 0x20, 0x28, 0x73, 0x6d, 0x6f, 0x6f, 0x74, 0x68, 0x29, 0x20,
  0x7b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x69, 0x6e,
  0x74, 0x34, 0x20, 0x63, 0x6d, 0x70, 0x20, 0x3d, 0x20, 0x69, 0x73, 0x6c,
  0x65, 0x73, 0x73, 0x28, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x29,
  0x28, 0x31, 0x36, 0x2e, 0x30, 0x66, 0x29, 0x2c, 0x20, 0x28, 0x66, 0x6c,
  0x6f, 0x61, 0x74, 0x34, 0x29, 0x28, 0x31, 0x36, 0x2e, 0x30, 0x66, 0x29,
  0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x77,
  0x68, 0x69, 0x6c, 0x65, 0x20, 0x28, 0x6e, 0x20, 0x3c, 0x20, 0x6d, 0x61,
  0x78, 0x29, 0x20, 0x7b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x20,
  0x61, 0x62, 0x20, 0x3d, 0x20, 0x61, 0x20, 0x2a, 0x20, 0x62, 0x3b, 0x0d,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x20, 0x63, 0x6d, 0x70, 0x56, 0x61,
  0x6c, 0x20, 0x3d, 0x20, 0x66, 0x6d, 0x61, 0x28, 0x61, 0x2c, 0x20, 0x61,
  0x2c, 0x20, 0x62, 0x20, 0x2a, 0x20, 0x62, 0x29, 0x3b, 0x0d, 0x0a, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x61, 0x20,
  0x3d, 0x20, 0x66, 0x6d, 0x61, 0x28, 0x61, 0x2c, 0x20, 0x61, 0x2c, 0x20,
  0x2d, 0x66, 0x6d, 0x61, 0x28, 0x62, 0x2c, 0x20, 0x62, 0x2c, 0x20, 0x2d,
  0x63, 0x61, 0x29, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x62, 0x20, 0x3d, 0x20, 0x66, 0x6d,
  0x61, 0x28, 0x32, 0x2c, 0x20, 0x61, 0x62, 0x2c, 0x20, 0x63, 0x62, 0x29,
  0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x72, 0x65, 0x73, 0x61, 0x20, 0x3d, 0x20, 0x61, 0x73, 0x5f,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x28, 0x28, 0x61, 0x73, 0x5f, 0x69,
  0x6e, 0x74, 0x34, 0x28, 0x61, 0x29, 0x20, 0x26, 0x20, 0x63, 0x6d, 0x70,
  0x29, 0x20, 0x7c, 0x20, 0x28, 0x61, 0x73, 0x5f, 0x69, 0x6e, 0x74, 0x34,
  0x28, 0x72, 0x65, 0x73, 0x61, 0x29, 0x20, 0x26, 0x20, 0x7e, 0x63, 0x6d,
  0x70, 0x29, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x72, 0x65, 0x73, 0x62, 0x20, 0x3d, 0x20,
  0x61, 0x73, 0x5f, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x28, 0x28, 0x61,
  0x73, 0x5f, 0x69, 0x6e, 0x74, 0x34, 0x28, 0x62, 0x29, 0x20, 0x26, 0x20,
  0x63, 0x6d, 0x70, 0x29, 0x20, 0x7c, 0x20, 0x28, 0x61, 0x73, 0x5f, 0x69,
  0x6e, 0x74, 0x34, 0x28, 0x72, 0x65, 0x73, 0x62, 0x29, 0x20, 0x26, 0x20,
  0x7e, 0x63, 0x6d, 0x70, 0x29, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x63, 0x6d, 0x70, 0x20,
  0x3d, 0x20, 0x69, 0x73, 0x6c, 0x65, 0x73, 0x73, 0x28, 0x63, 0x6d, 0x70,
  0x56, 0x61, 0x6c, 0x2c, 0x20, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34,
  0x29, 0x28, 0x31, 0x36, 0x2e, 0x30, 0x66, 0x29, 0x29, 0x3b, 0x0d, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x69,
  0x66, 0x20, 0x28, 0x21, 0x61, 0x6e, 0x79, 0x28, 0x63, 0x6d, 0x70, 0x29,
  0x29, 0x20, 0x62, 0x72, 0x65, 0x61, 0x6b, 0x3b, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x63, 0x6f, 0x75,
  0x6e, 0x74, 0x20, 0x2b, 0x3d, 0x20, 0x61, 0x73, 0x5f, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x28, 0x63, 0x6d, 0x70, 0x20, 0x26, 0x20, 0x61, 0x73,
  0x5f, 0x69, 0x6e, 0x74, 0x34, 0x28, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74,
  0x34, 0x29, 0x28, 0x31, 0x29, 0x29, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x6e, 0x2b, 0x2b,
  0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x7d, 0x0d,
  0x0a, 0x20, 0x20, 0x20, 0x7d, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x65,
  0x6c, 0x73, 0x65, 0x20, 0x7b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x77, 0x68, 0x69, 0x6c, 0x65, 0x20, 0x28, 0x6e, 0x20, 0x3c,
  0x20, 0x6d, 0x61, 0x78, 0x29, 0x20, 0x7b, 0x0d, 0x0a, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61,
  0x74, 0x34, 0x20, 0x61, 0x62, 0x20, 0x3d, 0x20, 0x61, 0x20, 0x2a, 0x20,
  0x62, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x20, 0x63, 0x6d,
  0x70, 0x56, 0x61, 0x6c, 0x20, 0x3d, 0x20, 0x66, 0x6d, 0x61, 0x28, 0x61,
  0x2c, 0x20, 0x61, 0x2c, 0x20, 0x62, 0x20, 0x2a, 0x20, 0x62, 0x29, 0x3b,
  0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x61, 0x20, 0x3d, 0x20, 0x66, 0x6d, 0x61, 0x28, 0x61, 0x2c, 0x20,
  0x61, 0x2c, 0x20, 0x2d, 0x66, 0x6d, 0x61, 0x28, 0x62, 0x2c, 0x20, 0x62,
  0x2c, 0x20, 0x2d, 0x63, 0x61, 0x29, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x62, 0x20, 0x3d,
  0x20, 0x66, 0x6d, 0x61, 0x28, 0x32, 0x2c, 0x20, 0x61, 0x62, 0x2c, 0x20,
  0x63, 0x62, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x69, 0x6e, 0x74, 0x34, 0x20, 0x63, 0x6d,
  0x70, 0x20, 0x3d, 0x20, 0x69, 0x73, 0x6c, 0x65, 0x73, 0x73, 0x28, 0x63,
  0x6d, 0x70, 0x56, 0x61, 0x6c, 0x2c, 0x20, 0x28, 0x66, 0x6c, 0x6f, 0x61,
  0x74, 0x34, 0x29, 0x28, 0x31, 0x36, 0x2e, 0x30, 0x66, 0x29, 0x29, 0x3b,
  0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x69, 0x66, 0x20, 0x28, 0x21, 0x61, 0x6e, 0x79, 0x28, 0x63, 0x6d,
  0x70, 0x29, 0x29, 0x20, 0x62, 0x72, 0x65, 0x61, 0x6b, 0x3b, 0x0d, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x63,
  0x6f, 0x75, 0x6e, 0x74, 0x20, 0x2b, 0x3d, 0x20, 0x61, 0x73, 0x5f, 0x66,
  0x6c, 0x6f, 0x61, 0x74, 0x34, 0x28, 0x63, 0x6d, 0x70, 0x20, 0x26, 0x20,
  0x61, 0x73, 0x5f, 0x69, 0x6e, 0x74, 0x34, 0x28, 0x28, 0x66, 0x6c, 0x6f,
  0x61, 0x74, 0x34, 0x29, 0x28, 0x31, 0x29, 0x29, 0x29, 0x3b, 0x0d, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x6e,
  0x2b, 0x2b, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x7d, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x7d, 0x0d, 0x0a, 0x0d, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x20, 0x72,
  0x65, 0x73, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x69, 0x66, 0x20,
  0x28, 0x73, 0x6d, 0x6f, 0x6f, 0x74, 0x68, 0x20, 0x21, 0x3d, 0x20, 0x30,
  0x29, 0x20, 0x7b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x69, 0x66, 0x20, 0x28, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x2e, 0x73,
  0x30, 0x20, 0x3e, 0x3d, 0x20, 0x30, 0x29, 0x0d, 0x0a, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x72, 0x65, 0x73,
  0x20, 0x3d, 0x20, 0x28, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x29,
  0x20, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x29, 0x20, 0x2b, 0x20, 0x28, 0x28,
  0x66, 0x6c, 0x6f, 0x61, 0x74, 0x34, 0x29, 0x28, 0x31, 0x2e, 0x30, 0x66,
  0x2c, 0x20, 0x31, 0x2e, 0x30, 0x66, 0x2c, 0x20, 0x31, 0x2e, 0x30, 0x66,
  0x2c, 0x20, 0x31, 0x2e, 0x30, 0x66, 0x29, 0x29, 0x20, 0x2d, 0x20, 0x6c,
  0x6f, 0x67, 0x32, 0x28, 0x6c, 0x6f, 0x67, 0x28, 0x66, 0x6d, 0x61, 0x28,
  0x72, 0x65, 0x73, 0x61, 0x2c, 0x20, 0x72, 0x65, 0x73, 0x61, 0x2c, 0x20,
  0x72, 0x65, 0x73, 0x62, 0x20, 0x2a, 0x20, 0x72, 0x65, 0x73, 0x62, 0x29,
  0x29, 0x20, 0x2a, 0x20, 0x30, 0x2e, 0x35, 0x29, 0x3b, 0x0d, 0x0a, 0x20,
  0x20, 0x20, 0x20, 0x7d, 0x0d, 0x0a, 0x0d, 0x0a, 0x0d, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x28, 0x69, 0x6e, 0x74, 0x20, 0x69,
  0x20, 0x3d, 0x20, 0x30, 0x3b, 0x20, 0x69, 0x20, 0x3c, 0x20, 0x34, 0x20,
  0x26, 0x26, 0x20, 0x69, 0x20, 0x2b, 0x20, 0x78, 0x20, 0x3c, 0x20, 0x77,
  0x69, 0x64, 0x74, 0x68, 0x3b, 0x20, 0x69, 0x2b, 0x2b, 0x29, 0x20, 0x7b,
  0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x69, 0x66, 0x20, 0x28, 0x73, 0x6d,
  0x6f, 0x6f, 0x74, 0x68, 0x20, 0x21, 0x3d, 0x20, 0x30, 0x29, 0x20, 0x7b,
  0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x69, 0x66,
  0x20, 0x28, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x5b, 0x69, 0x5d, 0x20, 0x3e,
  0x3d, 0x20, 0x30, 0x29, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x41, 0x5b, 0x69, 0x6e, 0x64, 0x65,
  0x78, 0x20, 0x2b, 0x20, 0x69, 0x5d, 0x20, 0x3d, 0x20, 0x28, 0x28, 0x66,
  0x6c, 0x6f, 0x61, 0x74, 0x29, 0x20, 0x63, 0x6f, 0x75, 0x6e, 0x74, 0x5b,
  0x69, 0x5d, 0x29, 0x20, 0x2b, 0x20, 0x31, 0x20, 0x2d, 0x20, 0x6c, 0x6f,
  0x67, 0x28, 0x6c, 0x6f, 0x67, 0x28, 0x66, 0x6d, 0x61, 0x28, 0x72, 0x65,
  0x73, 0x61, 0x5b, 0x69, 0x5d, 0x2c, 0x20, 0x72, 0x65, 0x73, 0x61, 0x5b,
  0x69, 0x5d, 0x2c, 0x20, 0x72, 0x65, 0x73, 0x62, 0x5b, 0x69, 0x5d, 0x20,
  0x2a, 0x20, 0x72, 0x65, 0x73, 0x62, 0x5b, 0x69, 0x5d, 0x29, 0x29, 0x20,
  0x2f, 0x20, 0x32, 0x29, 0x20, 0x2f, 0x20, 0x6c, 0x6f, 0x67, 0x28, 0x32,
  0x2e, 0x30, 0x66, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x7d,
  0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x65, 0x6c, 0x73, 0x65, 0x0d, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x41, 0x5b, 0x69, 0x6e,
  0x64, 0x65, 0x78, 0x20, 0x2b, 0x20, 0x69, 0x5d, 0x20, 0x3d, 0x20, 0x28,
  0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x29, 0x20, 0x63, 0x6f, 0x75, 0x6e,
  0x74, 0x5b, 0x69, 0x5d, 0x29, 0x3b, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x7d,
  0x0d, 0x0a, 0x7d, 0x0d, 0x0a, 0x2a, 0x2f, 0x0d, 0x0a
};
unsigned int float_cl_len = 3009;
