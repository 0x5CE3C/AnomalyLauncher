#include "main.h"
#include <d3dx9tex.h>
#include "bender_image.h"

IDirect3DTexture9* LoadTextureFromMemory(IDirect3DDevice9* device, const unsigned char* image_data, size_t image_size)
{
    IDirect3DTexture9* texture = nullptr;
    D3DXIMAGE_INFO info;

    HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(device, image_data, image_size, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, &info, NULL, &texture);

    if (FAILED(hr)) { return nullptr; }
    return texture;
}

static bool background_initialize = true;

void RenderImage(IDirect3DDevice9* device) {
    static IDirect3DTexture9* texture = LoadTextureFromMemory(device, bender, sizeof(bender));
    if (background_initialize)
    {
        //sn->con.AddLog("{00DD00}background() is init success");
        background_initialize = false; // Use false instead of 0 for clarity
    }
    if (!texture) {
        sn->con.AddLog("background loading: initialize error memory data");
        return;
    }

    // Get the current ImGui window size
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    ImVec2 position = ImVec2{ ImGui::GetWindowPos().x, ImGui::GetWindowPos().y}; // You can adjust this if you want to offset the image

    // Define the color (e.g., white with full opacity)
    ImU32 color = IM_COL32(92, 92, 92, 255); // RGBA format

    // Render the image with dynamic size and position
    ImGui::GetWindowDrawList()->AddImage((void*)texture, position, ImVec2{ position.x + windowSize.x + 3, position.y + windowSize.y + 3}, ImVec2{ 0, 0 }, ImVec2{ 1, 1 }, color);
}