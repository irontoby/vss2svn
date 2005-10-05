
SSAPIps.dll: dlldata.obj SSAPI_p.obj SSAPI_i.obj
	link /dll /out:SSAPIps.dll /def:SSAPIps.def /entry:DllMain dlldata.obj SSAPI_p.obj SSAPI_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del SSAPIps.dll
	@del SSAPIps.lib
	@del SSAPIps.exp
	@del dlldata.obj
	@del SSAPI_p.obj
	@del SSAPI_i.obj
