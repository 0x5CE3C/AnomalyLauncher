#include "main.h"
#include "gui.h"
#include <cmath>
#include "Fonts/imgui_fonts.h"
#include "TrayMenu.h"


ImFont* pFont;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device != NULL && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {		
		PostQuitMessage(0);
	}return 0;	


/*	case WM_SHOWWINDOW: {
		if (wideParameter == TRUE) {			
			// Устанавливаем фокус на ImGui
			SetFocus(window);
			SetForegroundWindow(window);
		}
	} return 0;	*/

/*	case WM_SETFOCUS:
        //ShowWindow(window, SW_SHOW);
		if (g_tray.IsWindowVisible())
		{
			//SetForegroundWindow(window);
			SetFocus(window);
		}
        break;*/
  

	case WM_TRAYICON:                              // +++ ДОБАВИТЬ +++
		return g_tray.HandleTrayMessage(wideParameter, longParameter);

/*    case WM_KILLFOCUS:        
        //ShowWindow(Window, SW_SHOW);
		SetForegroundWindow(window);  // Устанавливаем его на передний план
		SetFocus(window);             // Устанавливаем фокус на окно
        break;*/

	case WM_KILLFOCUS:
		// +++ ИЗМЕНИТЬ: не перехватывать фокус, когда UI скрыт в трей +++
		if (g_tray.IsWindowVisible())
		{
			SetForegroundWindow(window);
			SetFocus(window);
		}
		break;

	}	

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
	// Инициализация структуры WNDCLASSEX
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0L;
	windowClass.cbWndExtra = 0L;
	windowClass.hInstance = GetModuleHandle(NULL);
	// Загружаем иконку по ID ресурса (IDI_ICON1 == 121)
	HMODULE hModule = GetModuleHandle(nullptr);
	HICON hIcon = LoadIcon(hModule, MAKEINTRESOURCE(IDI_ICON1));	
	if (!hIcon) {
		// Fallback, если ресурс не найден (например, при сборке без .rc)
		hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	}
	windowClass.hIcon = hIcon;//NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = PROJECT_NAME;
	windowClass.hIconSm = NULL;

	RegisterClassEx(&windowClass);

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int posX = (screenWidth - WINDOW_WIDTH) / 2;
	int posY = (screenHeight - WINDOW_HEIGHT) / 2;

	window = CreateWindowEx(
		0,
		PROJECT_NAME,
		windowName,
		WS_POPUP,
		posX,
		posY,
		5,
		5,
		0,
		0,
		windowClass.hInstance,
		0
	);	

	ShowWindow(window, SW_HIDE);			
	UpdateWindow(window);	

	g_tray.Init(window, 1);   // +++ ДОБАВИТЬ: создать иконку в трее +++
}

void gui::DestroyHWindow() noexcept
{
	g_tray.Remove();            // +++ ДОБАВИТЬ: убрать иконку из трея +++
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);	
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	traceLastFunc("void gui::CreateImGui() noexcept");
	//IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	io.DeltaTime = 1.0f / 60.0f;

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImFontConfig config;
	config.FontDataOwnedByAtlas = false;

	//io.Fonts->AddFontFromMemoryTTF(devfont, sizeof(devfont), 17.f, &config, io.Fonts->GetGlyphRangesCyrillic());	

	pFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font, sizeof(font), 17.f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	IM_ASSERT(pFont != NULL);	

	sn_init();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void ReloadFont() {
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();

	float size = static_cast<float>(sn->set.font_size);

	pFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font, sizeof(font), size, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();	
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());


		// Handle viewports if enabled
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}		

		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

// Функция для получения цвета в зависимости от координаты y
ImU32 GetParticleColor(float y) {
	float normalizedY = (y - ImGui::GetWindowPos().y) / ImGui::GetWindowSize().y; // Нормализация
	float r = normalizedY; // Красный компонент
	float g = 0.0f; // Зеленый компонент
	float b = 1.0f - normalizedY; // Синий компонент
	return ImGui::GetColorU32(ImVec4(r, g, b, 1.0f)); // Возвращаем цвет
}

void RenderParticles()
{

	ImDrawList* drawList = ImGui::GetWindowDrawList();			//draws like foreground does but behind the imgui stuff. Ideal

	//particle properties
	static const int numParticles = 1024;
	static ImVec2 particlePositions[numParticles];
	static ImVec2 particleDistance;
	static ImVec2 particleVelocities[numParticles];

	static bool initialized = false;
	if (!initialized)
	{
		for (int i = 0; i < numParticles; ++i)
		{
			particlePositions[i] = ImVec2(
				ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * static_cast<float>(rand()) / RAND_MAX,
				ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * static_cast<float>(rand()) / RAND_MAX
			);

			particleVelocities[i] = ImVec2(
				static_cast<float>((rand() % 11) - 5),
				static_cast<float>((rand() % 11) - 5)
			);

		}

		initialized = true;
	}

	float sensitivity = 15.0f;

	ImVec2 cursorPos = ImGui::GetIO().MousePos;
	float deltaTime = ImGui::GetIO().DeltaTime;

	// Параметры логотипа Umbrella
	const int NUM_SEGMENTS = 8;                    // 8 сегментов, как в логотипе
	const float UMBRELLA_RADIUS = 100.0f;     // Радиус логотипа
	const float TARGET_RADIUS_MIN = 20.0f;  // Внутренний радиус дуги
	const float TARGET_RADIUS_MAX = 100.0f;  // Внешний радиус дуги
	const float MIN_DISTANCE_BETWEEN_PARTICLES = 8.0f; // Минимальное расстояние между частицами

	// Центр логотипа — позиция курсора
	float centerX = cursorPos.x;
	float centerY = cursorPos.y;

	// Вектор для хранения занятых позиций
	std::vector<ImVec2> occupiedPositions;

	// В цикле рендеринга частиц
	for (int i = 0; i < numParticles; ++i) {
		// Сохранение предыдущей позиции для расчёта инерции
		float prevX = particlePositions[i].x;
		float prevY = particlePositions[i].y;

		// Обновление позиции частиц на основе текущей скорости
		particlePositions[i].x += particleVelocities[i].x * deltaTime;
		particlePositions[i].y += particleVelocities[i].y * deltaTime;

		float AbsSpeed = 15.0f; // Абсолютная скорость
		float repelSpeed = 5.0f; // Скорость отталкивания
		float repelRadius = 20.0f; // Радиус, в пределах которого происходит отталкивание

		// Индивидуальные параметры для каждой частицы (инициализируются один раз при создании частицы)
		static float particleAngleOffset = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI; // Случайный начальный угол
		static float speedVariation = 0.7f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (1.5f - 0.5f))); // Вариация скорости 0.5–1.5
		static float repelVariation = 2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (4.0f - 2.0f))); // Множитель отталкивания 2–4

		// Рассчитываем расстояние до центра логотипа (курсора)
		float dx = centerX - particlePositions[i].x;
		float dy = centerY - particlePositions[i].y;
		float distanceToCenter = sqrt(dx * dx + dy * dy);

		// Флаг: находится ли частица в радиусе действия логотипа
		bool inUmbrellaZone = (distanceToCenter < UMBRELLA_RADIUS);

		// Параметры для отрисовки логотипа Umbrella (дуга)
		float umbrellaAlpha = 0.0f;
		float umbrellaSegmentAngle = 0.0f;

		if (inUmbrellaZone) {
			// Определяем, к какому сегменту принадлежит частица
			float particleAngle = atan2(dy, dx); // Угол от частицы к центру
			int segmentIndex = static_cast<int>((particleAngle + M_PI) / (2.0f * M_PI / NUM_SEGMENTS));
			segmentIndex = (segmentIndex + NUM_SEGMENTS) % NUM_SEGMENTS; // Корректировка для отрицательных углов

			// Целевая позиция на дуге сегмента
			float targetSegmentAngle = segmentIndex * (2.0f * M_PI / NUM_SEGMENTS);
			float angleOffset = static_cast<float>(i % 40) / 19.0f * (M_PI / 8.0f); // Распределение частиц по углу сегмента
			float targetAngle = targetSegmentAngle + angleOffset;

			// Радиус целевой позиции (между внутренним и внешним радиусом)
			float radiusRatio = static_cast<float>(i % 10) / 9.0f; // Распределение по радиусу
			float targetRadius = TARGET_RADIUS_MIN + radiusRatio * (TARGET_RADIUS_MAX - TARGET_RADIUS_MIN);

			float targetX = centerX + targetRadius * cos(targetAngle);
			float targetY = centerY + targetRadius * sin(targetAngle);

			// Проверка, не занята ли целевая позиция
			bool positionIsOccupied = false;
			for (const auto& occupiedPos : occupiedPositions) {
				float distanceToOccupied = sqrt(
					(occupiedPos.x - targetX) * (occupiedPos.x - targetX) +
					(occupiedPos.y - targetY) * (occupiedPos.y - targetY)
				);
				if (distanceToOccupied < MIN_DISTANCE_BETWEEN_PARTICLES) {
					positionIsOccupied = true;
					break;
				}
			}

			if (!positionIsOccupied) {
				// Вектор к целевой позиции
				float toTargetX = targetX - particlePositions[i].x;
				float toTargetY = targetY - particlePositions[i].y;
				float distanceToTarget = sqrt(toTargetX * toTargetX + toTargetY * toTargetY);

				// Сила притяжения к целевой точке (плавное движение)
				float attractionForce = 8.0f * deltaTime;

				if (distanceToTarget > 0.0001f) {
					toTargetX /= distanceToTarget;
					toTargetY /= distanceToTarget;

					// Обновляем позицию частицы, направляя её к целевой точке
					particlePositions[i].x += toTargetX * attractionForce;
					particlePositions[i].y += toTargetY * attractionForce;

					// Добавляем позицию в список занятых
					occupiedPositions.push_back(particlePositions[i]);
				}
			}
			else {
				// Если позиция занята, ищем альтернативную рядом
				float searchRadius = MIN_DISTANCE_BETWEEN_PARTICLES * 1.5f;
				bool foundFreePosition = false;

				for (int attempt = 0; attempt < 10; ++attempt) {
					float randomAngle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
					float newTargetX = targetX + cos(randomAngle) * searchRadius;
					float newTargetY = targetY + sin(randomAngle) * searchRadius;

					bool newPositionIsFree = true;
					for (const auto& occupiedPos : occupiedPositions) {
						float distanceToCheck = sqrt(
							(occupiedPos.x - newTargetX) * (occupiedPos.x - newTargetX) +
							(occupiedPos.y - newTargetY) * (occupiedPos.y - newTargetY)
						);
						if (distanceToCheck < MIN_DISTANCE_BETWEEN_PARTICLES) {
							newPositionIsFree = false;
							break;
						}
					}

					if (newPositionIsFree) {
						particlePositions[i].x = newTargetX;
						particlePositions[i].y = newTargetY;
						occupiedPositions.push_back({ newTargetX, newTargetY });
						foundFreePosition = true;
						break;
					}
				}

				if (!foundFreePosition) {
					// Если не нашли свободной позиции, оставляем частицу на месте
					particlePositions[i].x = prevX;
					particlePositions[i].y = prevY;
				}
			}

			// Вектор к целевой позиции
			float toTargetX = targetX - particlePositions[i].x;
			float toTargetY = targetY - particlePositions[i].y;
			float distanceToTarget = sqrt(toTargetX * toTargetX + toTargetY * toTargetY);

			// Сила притяжения к целевой точке (плавное движение)
			float attractionForce = 10.1f * deltaTime; // Усилили силу притяжения

			if (distanceToTarget > 0.0001f) {
				toTargetX /= distanceToTarget;
				toTargetY /= distanceToTarget;

				// Обновляем позицию частицы, направляя её к целевой точке
				particlePositions[i].x += toTargetX * attractionForce;
				particlePositions[i].y += toTargetY * attractionForce;
			}

			// Цвет частицы: чередование красного и белого для сегментов
			ImU32 particleColor;
			if (segmentIndex % 2 == 0) {
				particleColor = ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 0.9f)); // Красный
			}
			else {
				particleColor = ImGui::GetColorU32(ImVec4(0.0f, 0.7f, 0.7f, 0.9f)); // Белый
			}

			// Отрисовка частицы (увеличенный размер для лучшей видимости контура)
			drawList->AddCircleFilled(particlePositions[i], 2.0f, particleColor);

			// Отрисовка линий между частицами
			for (int j = i + 1; j < numParticles; ++j) {
				float distance = std::hypotf(particlePositions[j].x - particlePositions[i].x,
					particlePositions[j].y - particlePositions[i].y);
				float opacity = 0.5f - (distance / 55.0f); // Изменение прозрачности
				if (opacity > 0.0f) {
					if (segmentIndex % 2 == 0) {
						particleColor = ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, opacity)); // Красный
					}
					else {
						particleColor = ImGui::GetColorU32(ImVec4(0.0f, 0.7f, 0.7f, opacity)); // Белый
					}
					drawList->AddLine(particlePositions[i], particlePositions[j], particleColor);
				}
			}
			

			umbrellaSegmentAngle = particleAngle;
			umbrellaAlpha = max(0.0f, min(1.0f, 1.0f - (distanceToCenter / UMBRELLA_RADIUS)));

			// Цвет Umbrella: красный с прозрачностью
			ImU32 umbrellaColor = ImGui::GetColorU32(ImVec4(0.8f, 0.0f, 0.0f, umbrellaAlpha * 0.7f));

			// Параметры дуги
			float arcRadius = repelRadius * 0.9f;
			float segmentWidth = M_PI / 8.0f; // Ширина сегмента: 22.5 градуса

			float startAngle = umbrellaSegmentAngle - segmentWidth / 2.0f;
			float endAngle = umbrellaSegmentAngle + segmentWidth / 2.0f;

			// Количество точек для плавной дуги
			int numPoints = numParticles / NUM_SEGMENTS;
			std::vector<ImVec2> arcPoints;

			for (int k = 0; k <= numPoints; ++k) {
				float t = static_cast<float>(k) / static_cast<float>(numPoints);
				float angle = startAngle + t * (endAngle - startAngle);

				ImVec2 point;
				point.x = cursorPos.x + std::cos(angle) * arcRadius;
				point.y = cursorPos.y + std::sin(angle) * arcRadius;

				arcPoints.push_back(point);
			}

			// Акцент: маленькая точка на частице
			drawList->AddCircleFilled(particlePositions[i], 2.0f, umbrellaColor);
		}
		else {
			// Вне радиуса логотипа: логика из исходного кода (притяжение/отталкивание)

			// Рассчитываем расстояние до курсора
			float dx_cursor = cursorPos.x - particlePositions[i].x;
			float dy_cursor = cursorPos.y - particlePositions[i].y;
			float distance = sqrt(dx_cursor * dx_cursor + dy_cursor * dy_cursor);

			// Нормализуем вектор направления к курсору
			float normX = 0.0f, normY = 0.0f;
			if (distance > 0.0001f) {
				normX = dx_cursor / distance;
				normY = dy_cursor / distance;
			}

			// Логика притяжения с плавным направлением к курсору
			float attractionForce = 1.5f * deltaTime;
			particlePositions[i].x += normX * attractionForce;
			particlePositions[i].y += normY * attractionForce;

			// Флаг для отрисовки Umbrella?эффекта
			bool inRepelZone = (distance < repelRadius);			

			// Отрисовка линий между частицами
			for (int j = i + 1; j < numParticles; ++j) {
				float distance = std::hypotf(particlePositions[j].x - particlePositions[i].x,
					particlePositions[j].y - particlePositions[i].y);
				float opacity = 0.5f - (distance / 55.0f); // Изменение прозрачности
				if (opacity > 0.0f) {										
					ImU32 lineColor = ImGui::GetColorU32(ImVec4(0.0f, 0.5f, 0.5f, opacity));
					drawList->AddLine(particlePositions[i], particlePositions[j], lineColor);
				}
			}

			// Добавляем инерцию: частица продолжает движение по предыдущему вектору
			float inertiaX = particlePositions[i].x - prevX;
			float inertiaY = particlePositions[i].y - prevY;
			particlePositions[i].x += inertiaX * 0.7f;
			particlePositions[i].y += inertiaY * 0.7f;			
		}


		// Проверка границ окна
		if (particlePositions[i].x < ImGui::GetWindowPos().x)
			particlePositions[i].x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
		else if (particlePositions[i].x > ImGui::GetWindowPos().x + ImGui::GetWindowSize().x)
			particlePositions[i].x = ImGui::GetWindowPos().x;

		if (particlePositions[i].y < ImGui::GetWindowPos().y)
			particlePositions[i].y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
		else if (particlePositions[i].y > ImGui::GetWindowPos().y + ImGui::GetWindowSize().y)
			particlePositions[i].y = ImGui::GetWindowPos().y;


		// Очистка списка занятых позиций для следующего кадра
		occupiedPositions.clear();


		// Получение цвета для текущей частицы (вне зоны логотипа)
		ImU32 particleColor = GetParticleColor(particlePositions[i].y);

		// Рендеринг частиц вне зоны логотипа
		if (!inUmbrellaZone) {
			drawList->AddCircleFilled(particlePositions[i], 1.5f, particleColor);
		}
	}		

}


void gui::Render() noexcept
{
	auto thw = ThreadWrapper::create();
	//ImGui::SetNextWindowPos({ 0, 0 });
	//ImGui::SetNextWindowSize({ gui::WINDOW_WIDTH, gui::WINDOW_HEIGHT });

	// Применяем текущую тему (не меняется во время анимации)
	//ApplyCurrentTheme();

	ImGuiMenuTheme();
	//ImGuiMenuTheme_GrayRed();
	

	// +++ Трей: опрос событий + рендер popup +++
	g_tray.RenderPopup();

	// +++ Выход из трея +++
	if (g_tray.ShouldQuit())
	{
		g_tray.Remove();
		Log("[AnomalyLauncher]: Закрытие из трея.");
		ExitProcess(0);
	}

	// +++ UI скрыт — пропускаем отрисовку основного окна +++
	if (!g_tray.IsWindowVisible())
		return;


	static ImFuncs::TypewriterAnimator launcherAnimator(PROJECT_NAME, 2); // 3 кадра на символ

	ImGui::Begin(
		PROJECT_NAME,
		&isRunning,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
	);


	// +++ Запоминаем HWND viewport главного окна +++
	{
		ImGuiViewport* vp = ImGui::GetWindowViewport();
		if (vp && vp->PlatformHandle)
			g_tray.SetMainViewportHwnd((HWND)vp->PlatformHandle);
	}

	RenderImage(gui::device);

	// if(sn->particles.enabled)
	// {
	// 	thw->beginAsync(RenderParticles);
	// 	thw->joinAll();	
	// }
	
	
	size = ImGui::GetWindowSize();

	int width = strlen(PROJECT_NAME) * 4;
	ImGui::SameLine(ImGui::GetWindowWidth() / 2 - width);
	ImGui::PushFont(pFont);

	launcherAnimator.update();
	std::string launcher_name = launcherAnimator.getCurrent();

	//Log("%s", launcher_name.c_str());
	
	thw->beginThreadex(ImGui::TextWithColors, 5, u8"%s", launcher_name.c_str());

	ImGui::SameLine(ImGui::GetWindowWidth() - 30);
	
	if (ImGui::Button("--"))
	{
		//ShowWindow(GetActiveWindow(), SW_SHOWMINIMIZED);
		// +++ ИЗМЕНИТЬ: сворачивать в трей вместо минимизации окна +++
		g_tray.SetWindowVisible(false);
	}
	ImFuncs::FastDescr(u8"{FF0008}свернуть"); // R G B A
	ImGui::SameLine();
	if (ImGui::Button("X"))
	{		
		Log("[AnomalyLauncher]: Успешное закрытие.");
		g_tray.Remove();   // +++ ДОБАВИТЬ +++
		ExitProcess(0);
	}
	ImFuncs::FastDescr(u8"{FF0009}закрыть"); // R G B A                
	ImGui::PopFont();

	MyGui::renderImguiMenu();

	ImGui::End();

	// === Анимация смены темы ===
	/*if (isFading) {
		ImGuiIO& io = ImGui::GetIO();
		float fadeSpeed = 3.0f;
		themeFade += io.DeltaTime * fadeSpeed;
		themeFade = min(themeFade, 1.0f);

		if (themeFade >= 1.0f && sn->set.theme_type != pendingTheme) {
			sn->set.theme_type = pendingTheme;
			ApplyCurrentTheme();			
		}
	}

	if (themeFade > 0.0f) {
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("##fade", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs
		);
		ImDrawList* draw = ImGui::GetWindowDrawList();
		draw->AddRectFilled(
			ImVec2(0, 0),
			ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y),
			ImGui::GetColorU32(ImVec4(0, 0, 0, themeFade))
		);
		ImGui::End();
	}

	if (isFading && themeFade >= 1.0f) {
		static float postDelay = 0.0f;
		postDelay += ImGui::GetIO().DeltaTime;
		if (postDelay > 0.1f) {
			themeFade -= ImGui::GetIO().DeltaTime * 2.0f;
			themeFade = max(themeFade, 0.0f);
			if (themeFade == 0.0f) {
				isFading = false;
				postDelay = 0.0f;
			}
		}
	}*/

}