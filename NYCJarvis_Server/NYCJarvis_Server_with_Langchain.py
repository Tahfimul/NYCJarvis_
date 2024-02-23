import ctypes

clib = ctypes.CDLL("C:/Users/tahfi/VSCodeProjects/NYCJarvis/NYCJarvis_Server/build/libNYCJarvisServer.dll")

CustomServerHandle = ctypes.POINTER(ctypes.c_char)

clib.createCustomServer.argtypes = [ctypes.c_int]

clib.createCustomServer.restype = CustomServerHandle

clib.start.argtypes = [CustomServerHandle]

clib.update.argtypes = [CustomServerHandle, ctypes.c_size_t, ctypes.c_bool]

clib.sendInputString.argtypes = [CustomServerHandle, ctypes.c_char_p]

clib.getCurrentInputString.argtypes = [CustomServerHandle]

clib.getCurrentInputString.restype = ctypes.c_char_p

CustomServer_instance = clib.createCustomServer(60000)
clib.start(CustomServer_instance)
while True:
    clib.update(CustomServer_instance, -1, False)
    print(clib.getCurrentInputString(CustomServer_instance))
