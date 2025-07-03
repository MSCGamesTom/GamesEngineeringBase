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

// Include necessary Windows and DirectX headers
#include <Windows.h>
#include <string>
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <xaudio2.h>
#include <map>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <wrl/client.h>
#include <Xinput.h>
#include <math.h>

// Link necessary libraries
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "WindowsCodecs.lib")
#pragma comment(lib, "xinput.lib")

// Stop warnings about possible NULL values for buffer and backbuffer. This should work on any modern hardware.
#pragma warning( disable : 6387)

// Define the namespace to encapsulate the library's classes
namespace GamesEngineeringBase
{

	// Macros to extract mouse coordinates from LPARAM
#define CANVAS_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define CANVAS_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

	// Enum for mouse buttons
	enum MouseButton
	{
		MouseLeft = 0,
		MouseMiddle = 1,
		MouseRight = 2
	};

	// The Window class manages the creation and rendering of a window
	class Window
	{
	private:
		// Private member variables
		HWND hwnd;                               // Handle to the window
		HINSTANCE hinstance;                     // Handle to the application instance
		std::string name;                        // Window name/title
		ID3D11Device* dev;                       // Direct3D device
		ID3D11DeviceContext* devcontext;         // Direct3D device context
		IDXGISwapChain* sc;                      // Swap chain for double buffering
		ID3D11RenderTargetView* rtv;             // Render target view
		D3D11_VIEWPORT vp;                       // Viewport configuration
		ID3D11Buffer* buffer;                    // Buffer for pixel data
		ID3D11ShaderResourceView* srv;           // Shader resource view
		ID3D11PixelShader* ps;                   // Pixel shader
		ID3D11VertexShader* vs;                  // Vertex shader
		unsigned char* image;                    // Back buffer image data
		bool keys[256];                          // Keyboard state array
		int mousex;                              // Mouse X-coordinate
		int mousey;                              // Mouse Y-coordinate
		bool mouseButtons[3];                    // Mouse button states (left, middle, right)
		int mouseWheel;                          // Mouse wheel value
		unsigned int width = 0;                  // Window width
		unsigned int height = 0;                 // Window height
		unsigned int paddedDataSize = 0;         // Padding for backbuffer memory allocation

		// Static window procedure to handle window messages
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			Window* canvas = NULL;
			if (msg == WM_CREATE)
			{
				// On window creation, associate the Window instance with the HWND
				canvas = reinterpret_cast<Window*>(((LPCREATESTRUCT)lParam)->lpCreateParams);
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)canvas);
			} else
			{
				// Retrieve the Window instance associated with the HWND
				canvas = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			}
			if (canvas)
				return canvas->realWndProc(hwnd, msg, wParam, lParam);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		// Updates the internal mouse coordinates
		void updateMouse(int x, int y)
		{
			mousex = x;
			mousey = y;
		}

		// Instance-specific window procedure to handle messages
		LRESULT CALLBACK realWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			switch (msg)
			{
			case WM_DESTROY:
			case WM_CLOSE:
			{
				// Handle window close/destroy messages
				PostQuitMessage(0);
				exit(0);
				return 0;
			}
			case WM_KEYDOWN:
			{
				// Update key state to pressed
				keys[static_cast<unsigned int>(wParam)] = true;
				return 0;
			}
			case WM_KEYUP:
			{
				// Update key state to released
				keys[static_cast<unsigned int>(wParam)] = false;
				return 0;
			}
			case WM_LBUTTONDOWN:
			{
				// Handle left mouse button down
				updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
				mouseButtons[0] = true;
				return 0;
			}
			case WM_LBUTTONUP:
			{
				// Handle left mouse button up
				updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
				mouseButtons[0] = false;
				return 0;
			}
			case WM_RBUTTONDOWN:
			{
				// Handle right mouse button down
				updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
				mouseButtons[2] = true;
				return 0;
			}
			case WM_RBUTTONUP:
			{
				// Handle right mouse button up
				updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
				mouseButtons[2] = false;
				return 0;
			}
			case WM_MBUTTONDOWN:
			{
				// Handle middle mouse button down
				updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
				mouseButtons[1] = true;
				return 0;
			}
			case WM_MBUTTONUP:
			{
				// Handle middle mouse button up
				updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
				mouseButtons[1] = false;
				return 0;
			}
			case WM_MOUSEWHEEL:
			{
				// Handle mouse wheel movement
				updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
				mouseWheel += GET_WHEEL_DELTA_WPARAM(wParam);
				return 0;
			}
			case WM_MOUSEMOVE:
			{
				// Handle mouse movement
				updateMouse(CANVAS_GET_X_LPARAM(lParam), CANVAS_GET_Y_LPARAM(lParam));
				return 0;
			}
			default:
			{
				// Default message handling
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			}
		}

		// Processes window messages
		void pumpLoop()
		{
			MSG msg;
			ZeroMemory(&msg, sizeof(MSG));
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

	public:
		// Creates and initializes the window
		void create(unsigned int window_width, unsigned int window_height, const std::string window_name, bool window_fullscreen = false, int window_x = 0, int window_y = 0)
		{
			// Window class structure
			WNDCLASSEX wc;
			hinstance = GetModuleHandle(NULL);
			name = window_name;

			// Fill in the window class attributes
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

			// Register the window class
			RegisterClassEx(&wc);

			DWORD style;
			if (window_fullscreen)
			{
				// Configure fullscreen settings
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
				// Configure windowed mode settings
				width = window_width;
				height = window_height;
				style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX) | WS_VISIBLE;
			}

			// Set the process DPI awareness for proper scaling
			SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);

			// Adjust window rectangle to accommodate for window borders
			RECT wr = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
			AdjustWindowRect(&wr, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX), FALSE);

			// Create the window
			hwnd = CreateWindowEx(
				WS_EX_APPWINDOW,
				wname.c_str(),
				wname.c_str(),
				style,
				window_x,
				window_y,
				wr.right - wr.left,
				wr.bottom - wr.top,
				NULL,
				NULL,
				hinstance,
				this);

			// Display and focus the window
			ShowWindow(hwnd, SW_SHOW);
			SetForegroundWindow(hwnd);
			SetFocus(hwnd);

			// Initialize the swap chain description
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

			// Specify the DirectX feature level
			D3D_FEATURE_LEVEL fl;
			fl = D3D_FEATURE_LEVEL_11_0;

			// Create the Direct3D device and swap chain
			D3D11CreateDeviceAndSwapChain(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				0,
				&fl,
				1,
				D3D11_SDK_VERSION,
				&sd,
				&sc,
				&dev,
				NULL,
				&devcontext);

			// Get the back buffer from the swap chain
			ID3D11Texture2D* backbuffer;
			sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backbuffer);

			// Create the render target view
			dev->CreateRenderTargetView(backbuffer, NULL, &rtv);
			backbuffer->Release();

			// Configure the viewport
			vp.Width = (float)width;
			vp.Height = (float)height;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			// Set the viewport and render target
			devcontext->RSSetViewports(1, &vp);
			devcontext->OMSetRenderTargets(1, &rtv, NULL);

			// Calculate padding for GPU alignment
			unsigned int dataSize = width * height * 3;
			paddedDataSize = ((dataSize + 3) / 4) * 4;

			// Create buffer to hold the back buffer image
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = paddedDataSize * sizeof(unsigned char);
			bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			dev->CreateBuffer(&bufferDesc, nullptr, &buffer);

			//Create shader resource view
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
			srvDesc.BufferEx.FirstElement = 0;
			srvDesc.BufferEx.NumElements = paddedDataSize / 4;
			srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			dev->CreateShaderResourceView(buffer, &srvDesc, &srv);

			// Set the default states
			devcontext->OMSetBlendState(NULL, NULL, 0xffffffff);
			devcontext->OMSetDepthStencilState(NULL, 0);
			devcontext->RSSetState(NULL);
			devcontext->IASetInputLayout(NULL);
			devcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Vertex and pixel shader source code
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

			// Width is fixed so we can write it into the shader code at compile time
			std::string pixelShader = "ByteAddressBuffer buf : register(t0);\
            struct VSOut\
            {\
                float4 pos : SV_Position;\
            };\
            float4 PS(VSOut psInput) : SV_Target0\
            {\
				uint pixelIndex = (int(psInput.pos.y) * WIDTH) + int(psInput.pos.x); \
				uint offset = pixelIndex * 3;\
				uint inner = offset & 3;\
				uint baseAddress = offset & ~3;\
				uint data = 1;\
				if (inner == 0)\
				{\
					data = buf.Load(baseAddress) & 0xFFFFFF;\
				} else\
				{\
					data = ((buf.Load(baseAddress) >> (inner * 8)) | (buf.Load(baseAddress + 4) << ((4 - inner) * 8))) & 0xFFFFFF;\
				}\
				float r = (data & 0xFF) / 255.0;\
				float g = ((data >> 8) & 0xFF) / 255.0; \
				float b = ((data >> 16) & 0xFF) / 255.0; \
                return float4(r, g, b, 1.0f);\
            }";
			unsigned int startPos = 0;
			std::string widthStr = std::to_string(width);
			std::string widthConst = "WIDTH";
			startPos = static_cast<unsigned int>(pixelShader.find(widthConst, startPos));
			pixelShader.replace(startPos, widthConst.length(), widthStr);

			// Compile the shaders
			ID3DBlob* vshader;
			ID3DBlob* pshader;
			D3DCompile(
				vertexShader.c_str(),
				strlen(vertexShader.c_str()),
				NULL,
				NULL,
				NULL,
				"VS",
				"vs_5_0",
				0,
				0,
				&vshader,
				NULL);
			dev->CreateVertexShader(vshader->GetBufferPointer(), vshader->GetBufferSize(), NULL, &vs);

			D3DCompile(
				pixelShader.c_str(),
				strlen(pixelShader.c_str()),
				NULL,
				NULL,
				NULL,
				"PS",
				"ps_5_0",
				0,
				0,
				&pshader,
				NULL);
			dev->CreatePixelShader(pshader->GetBufferPointer(), pshader->GetBufferSize(), NULL, &ps);

			// Cleanup the shader build data
			vshader->Release();
			pshader->Release();

			// Set the shaders and shader resources
			devcontext->VSSetShader(vs, NULL, 0);
			devcontext->PSSetShader(ps, NULL, 0);
			devcontext->PSSetShaderResources(0, 1, &srv);

			// Allocate memory for the back buffer image data
			image = new unsigned char[paddedDataSize];
			clear(); // Clear the image data

			// Initialize input states
			memset(keys, 0, 256 * sizeof(bool));
			memset(mouseButtons, 0, 3 * sizeof(bool));

			// Initialize COM library for image loading
			HRESULT comResult;
			comResult = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		}

		// Processes input messages
		void checkInput()
		{
			pumpLoop();
		}

		// Returns a pointer to the back buffer image data
		unsigned char* backBuffer() const
		{
			return image;
		}

		// Draws a pixel at (x, y) with the specified RGB color
		void draw(int x, int y, unsigned char r, unsigned char g, unsigned char b)
		{
			int index = ((y * width) + x) * 3;
			image[index] = r;
			image[index + 1] = g;
			image[index + 2] = b;
		}

		// Draws a pixel at the specified pixel index with the given RGB color
		void draw(int pixelIndex, unsigned char r, unsigned char g, unsigned char b)
		{
			int index = pixelIndex * 3;
			image[index] = r;
			image[index + 1] = g;
			image[index + 2] = b;
		}

		// Draws a pixel at (x, y) using the color from the provided pixel array
		void draw(int x, int y, unsigned char* pixel)
		{
			int index = ((y * width) + x) * 3;
			image[index] = pixel[0];
			image[index + 1] = pixel[1];
			image[index + 2] = pixel[2];
		}

		// Clears the back buffer by setting all pixels to black
		void clear()
		{
			memset(image, 0, width * height * 3 * sizeof(unsigned char));
		}

		// Presents the back buffer to the screen
		void present()
		{
			// Update the texture data
			devcontext->UpdateSubresource(buffer, 0, nullptr, image, paddedDataSize, 0);

			// Clear the render target view
			float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // RGBA
			devcontext->ClearRenderTargetView(rtv, ClearColor);

			// Draw the vertices
			devcontext->Draw(3, 0);

			// Present the swap chain
			sc->Present(0, 0);

			// Process any pending messages
			pumpLoop();
		}

		// Returns the window's width
		unsigned int getWidth() const
		{
			return width;
		}

		// Returns the window's height
		unsigned int getHeight() const
		{
			return height;
		}

		// Provide raw access to back buffer
		// There are no checks done on this so any writes to this buffer should be within bounds
		// Can be used for screenshots
		unsigned char* getBackBuffer() const
		{
			return image;
		}

		// Checks if a specific key is currently pressed
		bool keyPressed(int key) const
		{
			return keys[key];
		}

		// Check if a mouse button is pressed. Takes a MouseButton enum
		bool mouseButtonPressed(MouseButton button) const
		{
			return mouseButtons[button];
		}

		// Returns the mouse x coordinate
		int getMouseX() const
		{
			return mousex;
		}

		// Returns the mouse y coordinate
		int getMouseY() const
		{
			return mousey;
		}

		// Returns the mouse wheel value
		int getMouseWheel() const
		{
			return mouseWheel;
		}

		// Gets the mouse X-coordinate relative to the window, accounting for zoom
		int getMouseInWindowX() const
		{
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(hwnd, &p);
			RECT rect;
			GetClientRect(hwnd, &rect);
			p.x = p.x - rect.left;
			p.x = static_cast<LONG>(p.x);
			return p.x;
		}

		// Gets the mouse Y-coordinate relative to the window, accounting for zoom
		int getMouseInWindowY() const
		{
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(hwnd, &p);
			RECT rect;
			GetClientRect(hwnd, &rect);
			p.y = p.y - rect.top;
			p.y = static_cast<LONG>(p.y);
			return p.y;
		}

		// Restricts the mouse cursor to the window's client area
		void clipMouseToWindow() const
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

		// Destructor to release resources
		~Window()
		{
			vs->Release();
			ps->Release();
			srv->Release();
			buffer->Release();
			rtv->Release();
			sc->Release();
			devcontext->Release();
			dev->Release();
			CoUninitialize();
		}
	};

	// FourCC codes for WAV file parsing (big-Endian)
#ifdef _XBOX
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

	// FourCC codes for WAV file parsing (little-endian)
#ifndef _XBOX
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

// The Sound class handles loading and playing WAV audio files
	class Sound
	{
	private:
		XAUDIO2_BUFFER buffer;                   // Audio buffer
		IXAudio2SourceVoice* sourceVoice[128];   // Array of source voices for playback
		int index;                               // Current index for source voices

		// Helper function to find a chunk in the WAV file (from documentation)
		HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
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
				if (bytesRead >= dwRIFFDataSize)
					return S_FALSE;
			}

			return S_OK;
		}

		// Helper function to read chunk data from the WAV file
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
		// Loads a WAV file into the audio buffer
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

			// Check the file type; it should be 'WAVE'
			FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
			DWORD filetype;
			ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
			if (filetype != fourccWAVE)
				return FALSE;

			// Read the 'fmt ' chunk to get the format
			FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
			ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition);

			// Read the 'data' chunk to get the audio data
			FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
			BYTE* pDataBuffer = new BYTE[dwChunkSize];
			ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

			// Fill the XAUDIO2_BUFFER structure
			buffer.AudioBytes = dwChunkSize;
			buffer.pAudioData = pDataBuffer;
			buffer.Flags = XAUDIO2_END_OF_STREAM;

			HRESULT hr;
			// Create multiple source voices for concurrent playback
			for (int i = 0; i < 128; i++)
			{
				if (FAILED(hr = xaudio->CreateSourceVoice(&sourceVoice[i], (WAVEFORMATEX*)&wfx)))
				{
					return false;
				}
			}

			// Submit the audio buffer to the first source voice
			if (FAILED(hr = sourceVoice[0]->SubmitSourceBuffer(&buffer)))
			{
				return false;
			}

			index = 0; // Reset the index
			return true;
		}

		// Plays the sound once
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

		// Plays the sound in an infinite loop (for music)
		void playMusic()
		{
			buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
			sourceVoice[index]->SubmitSourceBuffer(&buffer);
			sourceVoice[index]->Start(0);
		}
	};

	// The SoundManager class manages multiple Sound instances
	class SoundManager
	{
	private:
		IXAudio2* xaudio;                          // XAudio2 interface
		IXAudio2MasteringVoice* xaudioMasterVoice; // Mastering voice
		std::map<std::string, Sound*> sounds;      // Map of sounds
		Sound* music = NULL;                              // Music sound

		// Helper function to find a sound by filename
		Sound* find(std::string filename)
		{
			auto it = sounds.find(filename);
			if (it != sounds.end())
			{
				return it->second;
			}
			return NULL;
		}

	public:
		// Constructor that initializes XAudio2
		SoundManager()
		{
			HRESULT comResult;
			comResult = XAudio2Create(&xaudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
			comResult = xaudio->CreateMasteringVoice(&xaudioMasterVoice);
		}

		// Loads a sound effect
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

		// Plays a loaded sound effect
		void play(std::string filename)
		{
			Sound* sound = find(filename);
			if (sound != NULL)
			{
				sound->play();
			}
		}

		// Loads a music track
		void loadMusic(std::string filename)
		{
			music = new Sound();
			music->loadWAV(xaudio, filename);
		}

		// Plays the loaded music track
		void playMusic()
		{
			music->playMusic();
		}

		// Destructor to release resources
		~SoundManager()
		{
			xaudio->Release();
		}
	};

	// The Timer class provides high-resolution timing functionality
	class Timer
	{
	private:
		LARGE_INTEGER freq;   // Frequency of the performance counter
		LARGE_INTEGER start;  // Starting time

	public:
		// Constructor that initializes the frequency
		Timer()
		{
			QueryPerformanceFrequency(&freq);
			reset();
		}

		// Resets the timer
		void reset()
		{
			QueryPerformanceCounter(&start);
		}

		// Returns the elapsed time since the last reset in seconds. Note this should only be called once per frame as it resets the timer.
		float dt()
		{
			LARGE_INTEGER cur;
			QueryPerformanceCounter(&cur);
			float value = static_cast<float>(cur.QuadPart - start.QuadPart) / freq.QuadPart;
			reset();
			return value;
		}
	};

	// The Image class handles loading and manipulating images
	// This class is a bit of an exception in that the members are public. The reason for this is users may want to create procedural images.
	class Image
	{
	public:
		unsigned int width;       // Image width
		unsigned int height;      // Image height
		unsigned int channels;    // Number of color channels
		unsigned char* data;      // Pointer to image data

		// Default constructor
		Image()
		{
			width = 0;
			height = 0;
			channels = 0;
			data = nullptr;
		}
		Image(const Image&) = delete; // No copy constructor
		Image& operator=(const Image&) = delete;
		Image(Image&&) noexcept = default;  // Default move
		Image& operator=(Image&&) noexcept = default;

		// Loads an image from a file using WIC
		bool load(std::string filename)
		{
			Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
			HRESULT hr = ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
			if (FAILED(hr))
			{
				return false;
			}

			Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
			IWICStream* stream = NULL;
			factory->CreateStream(&stream);

			std::wstring wFilename = std::wstring(filename.begin(), filename.end());
			stream->InitializeFromFilename(wFilename.c_str(), GENERIC_READ);
			factory->CreateDecoderFromStream(stream, 0, WICDecodeMetadataCacheOnDemand, &decoder);

			Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
			decoder->GetFrame(0, &frame);

			frame->GetSize(&width, &height);
			WICPixelFormatGUID pixelFormat = { 0 };
			frame->GetPixelFormat(&pixelFormat);

			channels = 0;
			int isRGB = 0;
			// Determine the number of channels based on the pixel format
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
			unsigned int stride = (width * channels + 3) & ~3; // Align stride to 4 bytes

			if (stride == (width * channels))
			{
				// Copy pixels directly if stride matches
				frame->CopyPixels(0, stride, width * height * channels, data);
			} else
			{
				// Handle images with padded stride
				unsigned char* strideData = new unsigned char[stride * height];
				frame->CopyPixels(0, stride, width * height * channels, strideData);
				for (unsigned int i = 0; i < height; i++)
				{
					memcpy(&data[i * width * channels], &strideData[i * stride], width * channels * sizeof(unsigned char));
				}
				delete[] strideData;
			}

			if (isRGB == 0)
			{
				// Swap red and blue channels for BGR formats
				for (unsigned int i = 0; i < width * height; i++)
				{
					unsigned char p = data[i * channels];
					data[i * channels] = data[(i * channels) + 2];
					data[(i * channels) + 2] = p;
				}
			}
			return true;
		}

		// Returns a pointer to the pixel data at (x, y)
		// Note, the bounds are handled via clamping
		unsigned char* at(const unsigned int x, const unsigned int y) const
		{
			return &data[((min(y, height - 1) * width) + min(x, width  - 1)) * channels];
		}

		// Returns the alpha value of the pixel at (x, y)
		// Note, the bounds are handled via clamping
		unsigned char alphaAt(const unsigned int x, const unsigned int y) const
		{
			if (channels == 4)
			{
				return data[((min(y, height - 1) * width) + min(x, width - 1)) * channels + 3];
			}
			return 255;
		}

		// Returns a the colour specified by index at (x, y)
		// Note, the image bounds are handled via clamping, but the index is not checked
		unsigned char at(const unsigned int x, const unsigned int y, const unsigned int index) const
		{
			return data[(((min(y, height - 1) * width) + min(x, width - 1)) * channels) + index];
		}

		// Returns a pointer to the pixel data at (x, y)
		// Note, no checks performed on x and y coordinates
		unsigned char* atUnchecked(const unsigned int x, const unsigned int y) const
		{
			return &data[((y * width) + x) * channels];
		}

		// Returns the alpha value of the pixel at (x, y)
		// Note, no checks performed on x and y coordinates
		unsigned char alphaAtUnchecked(const unsigned int x, const unsigned int y) const
		{
			if (channels == 4)
			{
				return data[(((y * width) + x) * channels) + 3];
			}
			return 255;
		}

		// Checks if the image has an alpha channel
		bool hasAlpha() const
		{
			return channels == 4;
		}

		// Frees the allocated image data
		void free()
		{
			if (data != NULL)
			{
				delete[] data;
				data = NULL;
			}
		}

		// Destructor to free resources
		~Image()
		{
			free();
		}
	};

	// The XBoxController class represents a single Xbox controller
	class XBoxController
	{
	private:
		int ID = 0;                 // Controller ID
		XINPUT_STATE state = {};    // Current state of the controller
		float lX = 0;               // Left thumbstick X-axis value
		float lY = 0;               // Left thumbstick Y-axis value
		float rX = 0;               // Right thumbstick X-axis value
		float rY = 0;               // Right thumbstick Y-axis value
		float lT = 0;               // Left trigger value
		float rT = 0;               // Right trigger value

	public:
		// Constructor initializes the controller as inactive
		XBoxController() { ID = -1; }

		// Activates the controller with the given ID
		void activate(int _ID) { ID = _ID; }

		// Deactivates the controller
		void deactivate() { ID = -1; }

		// Updates the controller's state
		void update()
		{
			memset(&state, 0, sizeof(XINPUT_STATE));
			XInputGetState(ID, &state);

			// Process left thumbstick input
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

			// Process right thumbstick input
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

			// Process trigger inputs
			rT = static_cast<float>(state.Gamepad.bRightTrigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / static_cast<float>(255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			lT = static_cast<float>(state.Gamepad.bLeftTrigger - XINPUT_GAMEPAD_TRIGGER_THRESHOLD) / static_cast<float>(255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		}

		// Button state methods
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

		// Sets the vibration intensity for the left and right motors
		void vibrate(float l, float r)
		{
			unsigned short lV = min(static_cast<unsigned short>(l * 65535), 65535);
			unsigned short rV = min(static_cast<unsigned short>(r * 65535), 65535);
			XINPUT_VIBRATION vibration;
			memset(&vibration, 0, sizeof(XINPUT_VIBRATION));
			vibration.wLeftMotorSpeed = lV;
			vibration.wRightMotorSpeed = rV;
			XInputSetState(ID, &vibration);
		}

		// Returns the controller ID
		int getID()
		{
			return ID;
		}
	};

	// The XBoxControllers class manages multiple Xbox controllers
	class XBoxControllers
	{
	private:
		XBoxController controllers[XUSER_MAX_COUNT]; // Array of controllers

	public:
		// Constructor that probes for connected controllers
		XBoxControllers()
		{
			probeControllers();
		}

		// Returns the controller at the specified index
		XBoxController getPlayerController(int index)
		{
			return controllers[index];
		}

		// Returns the first active controller
		XBoxController getFirstPlayerController()
		{
			for (int i = 0; i < XUSER_MAX_COUNT; i++)
			{
				if (controllers[i].getID() > -1)
				{
					return controllers[i];
				}
			}
			// Return a default controller if none are connected
			return XBoxController();
		}

		// Checks if any controller is connected
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

		// Probes for connected controllers and updates their status
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