/*
MIT License

Copyright (c) 2024 MSc Games Engineering Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <Windows.h>
#include <string>
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <xaudio2.h>
#include <map>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <atlbase.h>
#include <Xinput.h>
#include <math.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "WindowsCodecs.lib")
#pragma comment(lib, "xinput.lib")

namespace GamesEngineeringBase
{


#define CANVAS_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define CANVAS_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

class Window
{
private:
	HWND hwnd;
	HINSTANCE hinstance;
	float invZoom;
	std::string name;
	ID3D11Device* dev;
	ID3D11DeviceContext* devcontext;
	IDXGISwapChain* sc;
	ID3D11RenderTargetView* rtv;
	D3D11_VIEWPORT vp;
	ID3D11Texture2D* tex;
	ID3D11ShaderResourceView* srv;
	ID3D11PixelShader* ps;
	ID3D11VertexShader* vs;
	unsigned char* image;
	bool keys[256];
	int mousex;
	int mousey;
	bool mouseButtons[3];
	int mouseWheel;
	int width;
	int height;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Window* canvas = NULL;
		if (msg == WM_CREATE)
		{
			canvas = reinterpret_cast<Window*>(((LPCREATESTRUCT)lParam)->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)canvas);
		} else
		{
			canvas = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}
		if (canvas) return canvas->realWndProc(hwnd, msg, wParam, lParam);
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	void updateMouse(int x, int y)
	{
		mousex = x;
		mousey = y;
	}
	LRESULT CALLBACK realWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			exit(0);
			return 0;
		}
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			exit(0);
			return 0;
		}
		case WM_KEYDOWN:
		{
			keys[(unsigned int)wParam] = true;
			return 0;
		}
		case WM_KEYUP:
		{
			keys[(unsigned int)wParam] = false;
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
			mouseButtons[0] = true;
			return 0;
		}
		case WM_LBUTTONUP:
		{
			updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
			mouseButtons[0] = false;
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
			mouseButtons[2] = true;
			return 0;
		}
		case WM_RBUTTONUP:
		{
			updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
			mouseButtons[2] = false;
			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
			mouseButtons[1] = true;
			return 0;
		}
		case WM_MBUTTONUP:
		{
			updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
			mouseButtons[1] = false;
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
			mouseWheel = mouseWheel + GET_WHEEL_DELTA_WPARAM(wParam);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
			return 0;
		}
		default:
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		}
	}
	void pumpLoop()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
public:
	void create(int window_width, int window_height, const std::string window_name, float zoom = 1.0f, bool window_fullscreen = false, int window_x = 0, int window_y = 0)
	{
		WNDCLASSEX wc;
		hinstance = GetModuleHandle(NULL);
		name = window_name;
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hinstance;
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;
		std::wstring wname = std::wstring(name.begin(), name.end());
		wc.lpszClassName = wname.c_str();
		wc.cbSize = sizeof(WNDCLASSEX);
		RegisterClassEx(&wc);
		DWORD style;
		if (window_fullscreen)
		{
			width = GetSystemMetrics(SM_CXSCREEN);
			height = GetSystemMetrics(SM_CYSCREEN);
			DEVMODE fs;
			memset(&fs, 0, sizeof(DEVMODE));
			fs.dmSize = sizeof(DEVMODE);
			fs.dmPelsWidth = (unsigned long)width;
			fs.dmPelsHeight = (unsigned long)height;
			fs.dmBitsPerPel = 32;
			fs.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			ChangeDisplaySettings(&fs, CDS_FULLSCREEN);
			style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
		} else
		{
			width = window_width;
			height = window_height;
			style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		}
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
		RECT wr = { 0, 0, (LONG)(width * zoom), (LONG)(height * zoom) };
		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
		hwnd = CreateWindowEx(WS_EX_APPWINDOW, wname.c_str(), wname.c_str(), style, window_x, window_y, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hinstance, this);
		invZoom = 1.0f / (float)zoom;
		ShowWindow(hwnd, SW_SHOW);
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		DXGI_SWAP_CHAIN_DESC sd;
		memset(&sd, 0, sizeof(DXGI_SWAP_CHAIN_DESC));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = window_fullscreen ? false : true;
		D3D_FEATURE_LEVEL fl;
		fl = D3D_FEATURE_LEVEL_11_0;
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &fl, 1, D3D11_SDK_VERSION, &sd, &sc, &dev, NULL, &devcontext);
		ID3D11Texture2D* backbuffer;
		sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);
		dev->CreateRenderTargetView(backbuffer, NULL, &rtv);
		backbuffer->Release();
		vp.Width = (float)width;
		vp.Height = (float)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		devcontext->RSSetViewports(1, &vp);
		devcontext->OMSetRenderTargets(1, &rtv, NULL);
		// Create texture
		D3D11_TEXTURE2D_DESC texdesc;
		memset(&texdesc, 0, sizeof(D3D11_TEXTURE2D_DESC));
		texdesc.Width = width * 3;
		texdesc.Height = height;
		texdesc.MipLevels = 1;
		texdesc.ArraySize = 1;
		texdesc.Format = DXGI_FORMAT_R8_UNORM;
		texdesc.SampleDesc.Count = 1;
		texdesc.Usage = D3D11_USAGE_DYNAMIC;
		texdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		texdesc.MiscFlags = 0;
		dev->CreateTexture2D(&texdesc, NULL, &tex);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc;
		srvdesc.Format = DXGI_FORMAT_R8_UNORM;
		srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvdesc.Texture2D.MostDetailedMip = 0;
		srvdesc.Texture2D.MipLevels = 1;
		dev->CreateShaderResourceView(tex, &srvdesc, &srv);
		devcontext->OMSetBlendState(NULL, NULL, 0xffffffff);
		devcontext->OMSetDepthStencilState(NULL, 0);
		devcontext->RSSetState(NULL);
		devcontext->IASetInputLayout(NULL);
		devcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		std::string vertexShader = "struct VSOut\
		{\
			float4 pos : SV_Position;\
		};\
		VSOut VS(uint vertexID : SV_VertexId)\
		{\
			VSOut output;\
			float2 texcoords = float2((vertexID << 1) & 2, vertexID & 2);\
			output.pos = float4((texcoords.x * 2.0f) - 1.0f, (-texcoords.y * 2.0f) + 1.0f, 0, 1.0f);\
			return output;\
		}";
		std::string pixelShader = "Texture2D tex : register(t0);\
		struct VSOut\
		{\
			float4 pos : SV_Position;\
		};\
		float4 PS(VSOut psInput) : SV_Target0\
		{\
			int3 texcoordsr = int3(psInput.pos.x * 3, psInput.pos.y, 0);\
			int3 texcoordsg = texcoordsr + int3(1, 0, 0);\
			int3 texcoordsb = texcoordsr + int3(2, 0, 0);\
			return float4(tex.Load(texcoordsb).r, tex.Load(texcoordsr).r, tex.Load(texcoordsg).r, 1.0f);\
		}";
		ID3DBlob* vshader;
		ID3DBlob* pshader;
		D3DCompile(vertexShader.c_str(), strlen(vertexShader.c_str()), NULL, NULL, NULL, "VS", "vs_5_0", 0, 0, &vshader, NULL);
		dev->CreateVertexShader(vshader->GetBufferPointer(), vshader->GetBufferSize(), NULL, &vs);
		D3DCompile(pixelShader.c_str(), strlen(pixelShader.c_str()), NULL, NULL, NULL, "PS", "ps_5_0", 0, 0, &pshader, NULL);
		dev->CreatePixelShader(pshader->GetBufferPointer(), pshader->GetBufferSize(), NULL, &ps);
		devcontext->VSSetShader(vs, NULL, 0);
		devcontext->PSSetShader(ps, NULL, 0);
		devcontext->PSSetShaderResources(0, 1, &srv);
		image = new unsigned char[width * height * 3];
		clear();
		memset(keys, 0, 256 * sizeof(bool));
		memset(mouseButtons, 0, 3 * sizeof(bool));
		HRESULT comResult;
		comResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	}
	void checkInput()
	{
		pumpLoop();
	}
	unsigned char* backBuffer()
	{
		return image;
	}
	void draw(int x, int y, unsigned char r, unsigned char g, unsigned char b)
	{
		int index = ((y * width) + x) * 3;
		image[index] = r;
		image[index + 1] = g;
		image[index + 2] = b;
	}
	void draw(int pixelIndex, unsigned char r, unsigned char g, unsigned char b)
	{
		int index = pixelIndex * 3;
		image[index] = r;
		image[index + 1] = g;
		image[index + 2] = b;
	}
	void draw(int x, int y, unsigned char *pixel)
	{
		int index = ((y * width) + x) * 3;
		image[index] = pixel[0];
		image[index + 1] = pixel[1];
		image[index + 2] = pixel[2];
	}
	void clear()
	{
		memset(image, 0, width * height * 3 * sizeof(unsigned char));
	}
	void present()
	{
		D3D11_MAPPED_SUBRESOURCE res;
		devcontext->Map(tex, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		memcpy(res.pData, image, width * height * 3 * sizeof(unsigned char));
		devcontext->Unmap(tex, 0);
		float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
		devcontext->ClearRenderTargetView(rtv, ClearColor);
		devcontext->Draw(3, 0);
		sc->Present(0, 0);
		pumpLoop();
	}
	int getWidth()
	{
		return width;
	}
	int getHeight()
	{
		return height;
	}
	bool keyPressed(int key)
	{
		return keys[key];
	}
	int getMouseInWindowX()
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		RECT rect;
		GetClientRect(hwnd, &rect);
		p.x = p.x - rect.left;
		p.x = (LONG)(p.x * invZoom);
		return p.x;
	}
	int getMouseInWindowY()
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		RECT rect;
		GetClientRect(hwnd, &rect);
		p.y = p.y - rect.top;
		p.y = (LONG)(p.y * invZoom);
		return p.y;
	}
	void clipMouseToWindow()
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		POINT ul;
		ul.x = rect.left;
		ul.y = rect.top;
		POINT lr;
		lr.x = rect.right;
		lr.y = rect.bottom;
		MapWindowPoints(hwnd, nullptr, &ul, 1);
		MapWindowPoints(hwnd, nullptr, &lr, 1);
		rect.left = ul.x;
		rect.top = ul.y;
		rect.right = lr.x;
		rect.bottom = lr.y;
		ClipCursor(&rect);
	}
	~Window()
	{
		vs->Release();
		ps->Release();
		srv->Release();
		tex->Release();
		rtv->Release();
		sc->Release();
		devcontext->Release();
		dev->Release();
		CoUninitialize();
	}
};

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

class Sound
{
private:
	XAUDIO2_BUFFER buffer;
	IXAudio2SourceVoice* sourceVoice[128];
	int index;
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition) // From Documentation
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());

		DWORD dwChunkType;
		DWORD dwChunkDataSize;
		DWORD dwRIFFDataSize = 0;
		DWORD dwFileType;
		DWORD bytesRead = 0;
		DWORD dwOffset = 0;

		while (hr == S_OK)
		{
			DWORD dwRead;
			if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			switch (dwChunkType)
			{
			case fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
					hr = HRESULT_FROM_WIN32(GetLastError());
				break;

			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
					return HRESULT_FROM_WIN32(GetLastError());
			}

			dwOffset += sizeof(DWORD) * 2;

			if (dwChunkType == fourcc)
			{
				dwChunkSize = dwChunkDataSize;
				dwChunkDataPosition = dwOffset;
				return S_OK;
			}

			dwOffset += dwChunkDataSize;

			if (bytesRead >= dwRIFFDataSize) return S_FALSE;

		}

		return S_OK;

	}
	HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());
		DWORD dwRead;
		if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}
public:
	bool loadWAV(IXAudio2* xaudio, std::string filename)
	{
		WAVEFORMATEXTENSIBLE wfx = { 0 };
		// Open the file
		HANDLE hFile = CreateFileA(
			filename.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		DWORD dwChunkSize;
		DWORD dwChunkPosition;
		//check the file type, should be fourccWAVE or 'XWMA'
		FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
		DWORD filetype;
		ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
		if (filetype != fourccWAVE)
			return S_FALSE;

		FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
		ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

		FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
		BYTE* pDataBuffer = new BYTE[dwChunkSize];
		ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

		buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
		buffer.pAudioData = pDataBuffer;  //buffer containing audio data
		buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		HRESULT hr;
		for (int i = 0; i < 128; i++)
		{
			if (FAILED(hr = xaudio->CreateSourceVoice(&sourceVoice[i], (WAVEFORMATEX*)&wfx)))
			{
				return false;
			}
		}
		if (FAILED(hr = sourceVoice[0]->SubmitSourceBuffer(&buffer)))
		{
			return false;
		}
		index = 0;
		return true;
	}
	void play()
	{
		sourceVoice[index]->SubmitSourceBuffer(&buffer);
		sourceVoice[index]->Start(0);
		index++;
		if (index == 128)
		{
			index = 0;
		}
	}
	void playMusic()
	{
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		sourceVoice[index]->SubmitSourceBuffer(&buffer);
		sourceVoice[index]->Start(0);
	}
};

class SoundManager
{
private:
	IXAudio2* xaudio;
	IXAudio2MasteringVoice* xaudioMasterVoice;
	std::map<std::string, Sound*> sounds;
	Sound* music;
	Sound* find(std::string filename)
	{
		std::map<std::string, Sound*>::iterator it = sounds.find(filename);
		if (it != sounds.end())
		{
			return it->second;
		}
		return NULL;
	}
public:
	SoundManager()
	{
		HRESULT comResult;
		comResult = XAudio2Create(&xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
		comResult = xaudio->CreateMasteringVoice(&xaudioMasterVoice);
	}
	void load(std::string filename)
	{
		if (find(filename) == NULL)
		{
			Sound* sound = new Sound();
			if (sound->loadWAV(xaudio, filename))
			{
				sounds[filename] = sound;
			}
		}
	}
	void play(std::string filename)
	{
		Sound* sound = find(filename);
		if (sound != NULL)
		{
			sound->play();
		}
	}
	void loadMusic(std::string filename)
	{
		music = new Sound();
		music->loadWAV(xaudio, filename);
	}
	void playMusic()
	{
		music->playMusic();
	}
	void freeSound(std::string filename)
	{
		Sound* sound = find(filename);
	}
	~SoundManager()
	{
		xaudio->Release();
	}
};

class Timer
{
private:
	LARGE_INTEGER freq;
	LARGE_INTEGER start;
public:
	Timer()
	{
		QueryPerformanceFrequency(&freq);
	}
	void reset()
	{
		QueryPerformanceCounter(&start);
	}
	float dt()
	{
		LARGE_INTEGER cur;
		QueryPerformanceCounter(&cur);
		return static_cast<double>(cur.QuadPart - start.QuadPart) / freq.QuadPart;
	}
};

class Image
{
public:
	unsigned int width;
	unsigned int height;
	int channels;
	unsigned char *data;
	bool load(std::string filename)
	{
		CComPtr<IWICImagingFactory> factory;
		factory.CoCreateInstance(CLSID_WICImagingFactory);
		CComPtr<IWICBitmapDecoder> decoder;
		IWICStream* stream = NULL;
		factory->CreateStream(&stream);
		std::wstring wFilename = std::wstring(filename.begin(), filename.end());
		stream->InitializeFromFilename(wFilename.c_str(), GENERIC_READ);
		factory->CreateDecoderFromStream(stream, 0, WICDecodeMetadataCacheOnDemand, &decoder);
		CComPtr<IWICBitmapFrameDecode> frame;
		decoder->GetFrame(0, &frame);
		frame->GetSize(&width, &height);
		WICPixelFormatGUID pixelFormat = { 0 };
		frame->GetPixelFormat(&pixelFormat);
		channels = 0;
		int isRGB = 0;
		if (pixelFormat == GUID_WICPixelFormat24bppBGR)
		{
			channels = 3;
		}
		if (pixelFormat == GUID_WICPixelFormat32bppBGRA)
		{
			channels = 4;
		}
		if (pixelFormat == GUID_WICPixelFormat24bppRGB)
		{
			channels = 3;
			isRGB = 1;
		}
		if (pixelFormat == GUID_WICPixelFormat32bppRGBA)
		{
			channels = 4;
			isRGB = 1;
		}
		if (channels == 0)
		{
			return false;
		}
		data = new unsigned char[width * height * channels];
		unsigned int stride = (width * channels + 3) & ~3;
		if (stride == (width * channels))
		{
			frame->CopyPixels(0, stride, width * height * channels, data);
		} else
		{
			unsigned char *strideData = new unsigned char[stride * height];
			frame->CopyPixels(0, stride, width * height * channels, strideData);
			for (int i = 0; i < (int)height; i++)
			{
				memcpy(&data[i * width * channels], &strideData[i * stride], width * channels * sizeof(unsigned char));
			}
			delete[] strideData;
		}
		if (isRGB == 0)
		{
			for (int i = 0; i < (int)(width * height); i++)
			{
				unsigned char p = data[i * channels];
				data[i * channels] = data[(i * channels) + 2];
				data[(i * channels) + 2] = p;
			}
		}
		return true;
	}
	unsigned char* at(int x, int y)
	{
		return &data[((y * width) + x) * channels];
	}
	unsigned char alphaAt(int x, int y)
	{
		if (channels == 4)
		{
			return data[(((y * width) + x) * channels) + 3];
		}
		return 255;
	}
	bool hasAlpha()
	{
		if (channels == 4)
		{
			return true;
		}
		false;
	}
	void free()
	{
		if (data != NULL)
		{
			delete[] data;
			data = NULL;
		}
	}
	~Image()
	{
		free();
	}
};

class XBoxController
{
private:
	int ID;
	XINPUT_STATE state;
	float lX;
	float lY;
	float rX;
	float rY;
	float lT;
	float rT;
public:
	XBoxController() { ID = -1; }
	void activate(int _ID) { ID = _ID; }
	void deactivate() { ID = -1; }
	void update()
	{
		memset(&state, 0, sizeof(XINPUT_STATE));
		XInputGetState(ID, &state);
		lX = state.Gamepad.sThumbLX;
		lY = state.Gamepad.sThumbLY;
		float lLen = sqrtf((lX * lX) + (lY * lY));
		lX = lX / lLen;
		lY = lY / lLen;
		if (lLen > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		{
			if (lLen > 32767)
			{
				lLen = 32767;
			}
			lLen = lLen - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			lLen = lLen / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		} else
		{
			lLen = 0;
		}
		lX = lX * lLen;
		lY = lY * lLen;
		rX = state.Gamepad.sThumbRX;
		rY = state.Gamepad.sThumbRY;
		float rLen = sqrtf((rX * rX) + (rY * rY));
		rX = rX / rLen;
		rY = rY / rLen;
		if (rLen > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		{
			if (rLen > 32767)
			{
				rLen = 32767;
			}
			rLen = rLen - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
			rLen = rLen / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		} else
		{
			rLen = 0;
		}
		rX = rX * rLen;
		rY = rY * rLen;
		rT = (float)(state.Gamepad.bRightTrigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (float)(255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		lT = (float)(state.Gamepad.bLeftTrigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / (float)(255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	}
	bool upPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) > 0); }
	bool downPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) > 0); }
	bool leftPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) > 0); }
	bool rightPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) > 0); }
	bool startPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_START) > 0); }
	bool backPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) > 0); }
	bool lThumbPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) > 0); }
	bool rThumbPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) > 0); }
	bool lShoulderPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) > 0); }
	bool rShoulderPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) > 0); }
	bool APressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) > 0); }
	bool BPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_B) > 0); }
	bool XPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_X) > 0); }
	bool YPressed() { return ((state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) > 0); }
	void vibrate(float l, float r)
	{
		unsigned short lV = min((unsigned short)(l * 65535), 65535);
		unsigned short rV = min((unsigned short)(r * 65535), 65535);
		XINPUT_VIBRATION vibration;
		memset(&vibration, 0, sizeof(XINPUT_VIBRATION));
		vibration.wLeftMotorSpeed = lV;
		vibration.wRightMotorSpeed = rV;
		XInputSetState(ID, &vibration);
	}
	int getID()
	{
		return ID;
	}
};

class XBoxControllers
{
private:
	XBoxController controllers[XUSER_MAX_COUNT];
public:
	XBoxControllers()
	{
		probeControllers();
	}
	XBoxController getPlayerController(int index)
	{
		return controllers[index];
	}
	XBoxController getFirstPlayerController()
	{
		for (int i = 0; i < XUSER_MAX_COUNT; i++)
		{
			if (controllers[i].getID() > -1)
			{
				return controllers[i];
			}
		}
	}
	bool hasController()
	{
		for (int i = 0; i < XUSER_MAX_COUNT; i++)
		{
			if (controllers[i].getID() > -1)
			{
				return true;
			}
		}
		return false;
	}
	void probeControllers()
	{
		for (int i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XINPUT_STATE state;
			memset(&state, 0, sizeof(XINPUT_STATE));
			if (XInputGetState(i, &state) == 0)
			{
				controllers[i].activate(i);
			} else
			{
				controllers[i].deactivate();
			}
		}
	}
};

}