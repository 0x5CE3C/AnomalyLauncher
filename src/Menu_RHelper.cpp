#include "main.h"
using namespace ImGui;
using namespace ImFuncs;

void enable_directplay()
{
	exec("dism /online /Enable-Feature /FeatureName:DirectPlay /All");
}

struct CurlError {
	int code;
	std::string name;
	std::string meaning;
	std::string fix_simple;  // Простой совет
};

static std::vector<CurlError> curl_errors = {
	//{0,  "CURLE_OK", u8"Всё прошло успешно", u8"Всё в порядке — ошибка не возникла."},
	// {1,  "CURLE_UNSUPPORTED_PROTOCOL", u8"Протокол не поддерживается", u8"Попробуйте заменить ftp:// или другой адрес на https://"},
	{2,  "CURLE_FAILED_INIT", u8"cURL не смог запуститься", u8"Перезапустите программу или компьютер"},
	{3,  "CURLE_URL_MALFORMAT", u8"Ссылка написана неправильно", u8"Проверьте, нет ли опечаток в адресе сайта"},
	{6,  "CURLE_COULDNT_RESOLVE_HOST", u8"Не найден сайт", u8"Проверьте интернет и имя сайта (например, ya.ru)"},
	{7,  "CURLE_COULDNT_CONNECT", u8"Не удалось подключиться", u8"Сайт может быть временно недоступен. Попробуйте позже"},
	{22, "CURLE_HTTP_RETURNED_ERROR", u8"Сервер вернул ошибку", u8"Ссылка недействительна"},
	{28, "CURLE_OPERATION_TIMEDOUT", u8"Сервер слишком долго не отвечает", u8"Проверьте интернет или попробуйте снова через минуту"},
	//{35, "CURLE_SSL_CONNECT_ERROR", u8"Проблема с безопасным подключением", u8"Сайт использует устаревшую защиту. Попробуйте открыть его в браузере"},
	{47, "CURLE_TOO_MANY_REDIRECTS", u8"Сайт бесконечно перенаправляет", u8"Проблема на стороне сайта. Подождите или попробуйте другую ссылку"},
	{52, "CURLE_GOT_NOTHING", u8"Сервер ничего не прислал", u8"Сайт не отвечает. Проверьте, работает ли он в браузере"},
	{56, "CURLE_RECV_ERROR", u8"Ошибка при получении данных", u8"Соединение оборвалось. Проверьте интернет и повторите"},
	//{60, "CURLE_PEER_FAILED_VERIFICATION", u8"Сертификат сайта ненадёжен", u8"Сайт может быть поддельным. Не вводите пароли. Проверьте адрес"},
	{78, "CURLE_REMOTE_FILE_NOT_FOUND", u8"Файл не найден", u8"Файл удалён или перемещён. Проверьте ссылку"},
};

void RenderCurlErrorTreeSimple() {
	static char filter_buffer[64] = "";

	if (ImGui::TreeNode(u8"Ошибки cURL(Загрузчик файлов\\http-клиент)")) {
		ImGui::Text(u8"Введите код ошибки: например, '22'");
		ImGui::InputText(u8"Фильтр", filter_buffer, IM_ARRAYSIZE(filter_buffer));
		ImGui::Separator();

		if (ImGui::BeginTable("curl_errors_simple", 3,
			ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {

			ImGui::TableSetupColumn(u8"Код", ImGuiTableColumnFlags_WidthFixed, 50.0f);
			//ImGui::TableSetupColumn(u8"Ошибка", ImGuiTableColumnFlags_WidthStretch, 0.2f);
			ImGui::TableSetupColumn(u8"Что случилось", ImGuiTableColumnFlags_WidthStretch, 0.3f);
			ImGui::TableSetupColumn(u8"Что делать", ImGuiTableColumnFlags_WidthStretch, 0.4f);
			ImGui::TableHeadersRow();

			std::string filter = filter_buffer;
			std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

			for (const auto& err : curl_errors) {
				std::string name = err.name;
				std::string meaning = err.meaning;
				std::string fix = err.fix_simple;
				std::transform(name.begin(), name.end(), name.begin(), ::tolower);
				std::transform(meaning.begin(), meaning.end(), meaning.begin(), ::tolower);
				std::transform(fix.begin(), fix.end(), fix.begin(), ::tolower);

				bool matches = filter.empty() ||
					std::to_string(err.code).find(filter) != std::string::npos ||
					name.find(filter) != std::string::npos ||
					meaning.find(filter) != std::string::npos ||
					fix.find(filter) != std::string::npos;

				if (!matches) continue;

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d", err.code);
				//ImGui::TableSetColumnIndex(1);
				//ImGui::Text("%s", err.name.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", err.meaning.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::TextWrapped("%s", err.fix_simple.c_str());
			}

			ImGui::EndTable();
		}

		ImGui::TreePop();
	}
}

void Menu_render_helper() 
{			


	if (ImGui::TreeNode(u8"Инструкция по установке сборки Purge of Zone(для игроков)"))
	{
		TextWithColors(u8"\
{AAAAFF}===========================================/\
{FFFFFF}\
1) Заходи в {FFFF00}\"{AAFFFF}Дополнительные компоненты{FFFF00}\"{FFFFFF}(Находится в этой же вкладке). /Скачай и Установи по порядку все нужные компоненты//\
2) Заходи в {FFFF00}\"{AAFFFF}Обновления -> Сборка{FFFF00}\" {FFFFFF}и нажимай {FFFF00}\"{AAFFFF}Доступно обновление -> Установить{FFFF00}\"{FFFFFF},/ Так же ты можешь Включить кнопку \"{AAFFFF}Обновить сборку{FFFFFF}\" и нажать только на неё 1 раз.//\
{AAFFFF}После этого Anomaly, MO2 и Сборка установятся автоматически!/{FFFFFF}\
{AAAAFF}===========================================");

		ImGui::TreePop();
		ImGui::Spacing();
	}

	if (ImGui::TreeNode(u8"Инструкция по установке кастомной сборки(для игроков)"))
	{
		TextWithColors(u8"\
{AAAAFF}===========================================/\
{FFFFFF}\
1) Положи конфиг файл AnomalyLauncher.ini в каталог AnomalyLauncher и перезапусти лаунчер.//\
2) Заходи в {FFFF00}\"{AAFFFF}Дополнительные компоненты{FFFF00}\"{FFFFFF}(Находится в этой же вкладке).//Скачай и Установи по порядку все нужные компоненты//\
3) Заходи в {FFFF00}\"{AAFFFF}Обновления -> Сборка{FFFF00}\" {FFFFFF}и нажимай {FFFF00}\"{AAFFFF}Доступно обновление -> Установить{FFFF00}\"/{FFFFFF} или ты можешь Включить кнопку \"{AAFFFF}Обновить сборку{FFFFFF}\" и нажать только на неё 1 раз.//\
{AAFFFF}После этого Anomaly, MO2 и Сборка установятся автоматически!/{FFFFFF}\
{AAAAFF}===========================================");

		ImGui::TreePop();
		ImGui::Spacing();
	}

	if (ImGui::TreeNode(u8"Универсальный режим(подробное описание для разработчиков)"))
	{		
		TextWithColors(u8"\
{FFFF00}Универсальный режим - инструмент для разработчиков кастомных сборок, позволяет легко интегрировать и обновлять моды в ваших сборках на базе Anomaly.//\
 При включении и генерации в списке файлов меняет флаг с \"pz\"(сборка Purge of Zone) на \"mp\"(от ModPack - Имя профиля вашей сборки)//");

		TextWithColors(u8"Для использования требуется://\
			1) Файловое хранилище\\обменник или любой Бесплатный хостинг с хорошим файловым менеджером с поддержкой прямых ссылок к файлам.//\
			2) Объектное хранилище S3.//\
			Например, в своей сборке я использую://\
\
		{AAFFFF}> бесплатный хостинг на ucoz для прямых ссылок к ключевым файлам files_list.ini(список файлов) и modpack_changelist.ini(список изменений в сборке).//\
		> сервис firstvds->Объектное хранилище S3(хранилище модов в zip - архивах).//");

		TextWithColors(u8"Приложения://\
1) CyberDuck(для подключения к S3 хранилищу, загрузки больших архивов, позволяет сразу генерировать несколько ссылок на скачивание).//\
2) Notepad++ - Необязательно(для нормального отображения ini-синтаксиса и ручного редактирования списка файлов).//\
Архиватор не требуется, для запаковки в .zip используется внешний распаковщик 7za.exe! Если случайно удалили - он восстановится из ресурсов лаунчера автоматически.//");

TextWithColors(u8"Этап 1: Подготовка файловой структуры//\
В первую очередь должна быть строгая базовая файловая структура(ниже пример)://\
Путь к модулю <- Модуль, краткое описание://\
{AAFFBB}\"E:\\AnomalyPZ\\AnomalyLauncher\\\"{FFFFFF} <- extractor, здесь распаковщик 7za.exe(7zip) и конфиги лаунчера./\
{AAFFBB}\"E:\\AnomalyPZ\\\"{FFFFFF} <- Anomaly [ папки bin, db, tools, файлы fsgame.ltx и т.д. ]/\
{AAFFBB}\"E:\\AnomalyPZ\\MO2\"{FFFFFF} <- Mod Organizer 2[профиль сборки тут же в \\profiles\\ ] > файлы заменятся из - за разных хешей при обновлении./\
{AAFFBB}\"E:\\AnomalyPZ\\MO2\\mods\\{FFFFFF} <- modpack (здесь хранятся ваши моды).//");
TextWithColors(u8"{AAFFBB}\"E:\\AnomalyPZ\\\"{FFFFFF} <- здесь находится лаунчер.//");

TextWithColors(u8"Этап 2: Настройка в лаунчере//\
1) Заходим в вкладку \"Листы\"->Включаем \"Универсальный режим\". Указываем \"имя профиля сборки\" из \"\\MO2\\profiles\\\"(это имя каталога будет ставиться при \"Патче путей\" MO2 в файле ModOrganizer.ini во время обновления).//\
\
2) В этой же вкладке заходим в \"Генератор конфигурации\". Включаем \"Проверки хешей\"(обе кнопки) и нажимаем \"Сгенерировать список\". Детальный статус выполнения отображается в Консоли.//\
3) Обновляем список: заходим в \"Список файлов\"->\"Обновить Лист\". Закрываем окно, нажимаем\"Сохранить настройки\".//\
\
На этом этапе Список файлов готов!//");
			
TextWithColors(u8"Этап 3: запаковка модулей, загрузка на сервер и импорт ссылок на скачивание://\
1) Запаковываем Anomaly, MO2 и Моды в отдельные архивы при помощи кнопок в разделе \"Отладка\".(Можно сразу тыкать все 3, кнопки создают задачи, отслеживаемые в реальном времени)//");

	TextWithColors(u8"{AAFFFF}Файлы Модулей запаковываются в отдельные архивы рядом с настоящими файлами.//Почему так: при докачке игроку не придется заново скачивать каждый раз архив, например Anomaly весом в 16 Гб. Игрок скачает только то, что повреждено или отсутствует.//\
{FFFF00}Сторонние моды которых нету в списке файлов при обновлении автоматически удаляются(моды, которые игрок самостоятельно установил поверх сборки и повреждённые если не совпадает хеш-сумма).//");
		
	TextWithColors(u8"\
2) Закачиваем архивы в ваше Облачное хранилище, желательно по такой же структуре как в папке с игрой(чтобы не теряться).//\
3) Создаем ссылки на скачивание и копируем в любой текстовый файл.//\
4) При помощи кнопки \"Обновить URL из файла\" в \"Листах\" импортируем ссылки из вашего файла в список файлов(заполняются последние пустые кавычки \"\").//\
		На этом этапе Список файлов сразу сохраняется в кодировке utf - 8, стандартной для правильного захвата кириллицы по ссылке.//");

	TextWithColors(u8"Этап 4: Загрузка ключевых файлов на хостинг и раздача://\
1) После импорта осталось загрузить \"список файлов\" на ваш бесплатный хостинг и скопировать прямую ссылку в \"Универсальный режим\", жмём \"Сохранить настройки\".//\
2) Формируем \"список изменений\"(этот файл отвечает за появление клавиши \"обновлений\" из-за разной версии при сравнении).//");

Text(u8"Пример отображения(\"modpack_changelist.ini\"):");

		Separator();
		BeginGroup();
		TextWithColors(u8"\
{AAFFFF}Версия: 0.0.0.8\n\
{FFFF00}Изменения://\
{AAFFFF}Добавлены новые модели {FFFFFF}сталкеров. /\
{AAFFFF}Исправлен какой - то баг.//\
{AAFFFF}Исправлен {AAFFBB}другой {FFFFFF}баг.//\
{65AAFFFF}Полупрозрачный текст.");
		EndGroup();
		Separator();
		NewLine();

		Text(u8"Как выглядит конфиг:");
		Separator();
		BeginGroup();
		TextWrapped(u8"\
#[modpack]\n\
#modpack_latest_version = 0.0.0.7\n\
#\n\
#modpack_latest_changelist = //\n\
[modpack]\n\
modpack_latest_version = 0.0.0.8\n\
\n\
modpack_latest_changelist = //\n\
{AAFFFF}Добавлены новые модели {FFFFFF}сталкеров. /\n\
{AAFFFF}Исправлен какой - то баг.//\n\
{AAFFFF}Исправлен {AAFFBB}другой {FFFFFF}баг.//\n\
{65AAFFFF}Полупрозрачный текст.");
		EndGroup();
		Separator();
		NewLine();

		Text(u8"синтаксис:");
		Separator();
		BeginGroup();
		TextWrapped(u8"\
# - Комментарий. Строка после # игнорируется.\n\
[modpack] - лаунчер захватывает эту секцию.\n\
modpack_latest_version = текст - определяет версию сборки(всё что после = это строка).Если строки не совпадают, лаунчер разблокирует кнопки обновления.\n\
modpack_latest_changelist = // - список изменений. Где // - переход на следующую строку, / - захватывает следующую строку.\n\
{AAFFFF} - цвета в hex - формате(RGB), { AA - красный оттенок, FF - зелёный, FF - синий }.\n\
{65AAFFFF} - цвета в hex - формате(ARGB), { 65 - прозрачность, AA - красный оттенок, FF - зелёный, FF - синий }.");
		EndGroup();
		Separator();

TextWithColors(u8"//3)Так же загружаем его на хостинг и копируем прямую ссылку в \"Универсальный режим\", не забудьте \"Сохранить настройки\".//");


		TextWithColors(u8"Теперь игрокам достаточно отправить 1 конфиг файл \\AnomalyLauncher\\AnomalyLauncher.ini 1 раз, далее лаунчер при каждом запуске будет закачивать список файлов и список изменений.//\
\
			Если версии не совпадают ставится флаг, кнопка обновления разблокируется. Даже если опция \"скачивать список файлов при запуске\" отключена, то после нажатия кнопки \"Установить\" или \"Обновить сборку\" список файлов принудительно скачается.");		
		
		
		ImGui::TreePop();
		ImGui::Spacing();
	}	

	if (ImGui::TreeNode(u8"Не запускается Anomaly"))
	{		
		ImGui::BeginGroup();
		{
			if(IsAppRunningAsAdminMode() == true)
			{
				HelpDescr(u8"{FF0000}Сложная операция, для обновления статуса перезагрузите лаунчер");
				ImGui::SameLine();
				ImGui::Text(u8"статус DirectPlay: ");
				ImGui::SameLine();
				if (!directPlayChecked) {
					ImGui::TextDisabled("Проверка...");
				}
				else if (directPlayEnabled) {
					ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), u8"Включен");
				}
				else {
					ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), u8"Отключен");
				}

				if (!directPlayEnabled)
				{
					if (pMenu->AddStaticButton(u8"Включить DirectPlay"))
					{
						sn->currentTab = TabId::CONSOLE;
						auto thw = ThreadWrapper::create();
						thw->beginThread(enable_directplay);
						thw->detachAll();
					}
				}	
			}
			else
			{
				ImGui::TextWrapped(u8"Для разблокировки дополнительных функций перезапустите лаунчер от имени администратора");
			}
		}
		ImGui::EndGroup();

/*		-------------------------- -
			Ошибка
			-------------------------- -
			Unhandled USVFS proxy exception(line 112) : Отказано в доступе.
			-------------------------- -
			ОК
			-------------------------- -*/



		ImGui::TreePop();
		ImGui::Spacing();
	}	

	ImVec4 text_color = ImColor(250,250,0, 250);//RGBA	

	ImGui::PushStyleColor(ImGuiCol_Text, text_color);

	if (ImGui::TreeNode(u8"Дополнительные компоненты"))
	{
		string comp_path = GetExePath() + "\\" + "components" + "\\";

		if (!fsm->FolderExist(comp_path.c_str()))
		{
			fsm->CreatefolderRecursive(comp_path.c_str());
		}

		string directx_url = "https://download.microsoft.com/download/1/7/1/1718ccc4-6315-4d8e-9543-8e28a4e18c4c/dxwebsetup.exe";
		string directx_fname = "dxwebsetup.exe";
		string directx = comp_path + directx_fname;

		string msvcr_url = "https://aka.ms/vs/17/release/vc_redist.x64.exe";
		string msvcr_fname = "VC_redist.x64.exe";
		string msvcr = comp_path + msvcr_fname;

		string openal_url = "https://openal.org/downloads/oalinst.zip";
		string openal_fname = "oalinst.exe";
		string openal = comp_path + openal_fname;
		string openal_unpack_dir = "components";
		string openal_zip_name = "oalinst.zip";
		string openal_zip_path = GetExePath() + "\\" + "components" + "\\" + openal_zip_name;

		if (fsm->FileExist(directx.c_str()))
		{
			if (pMenu->AddStaticButton(u8"Установить библиотеки DirectX"))
			{
				ShellExecuteA(0, "open", directx.c_str(), 0, 0, SW_SHOWNORMAL);
			}
		}
		else
		{
			if (pMenu->AddStaticButton(u8"Скачать библиотеки DirectX"))
			{
				executor->AddTask([directx_url, directx]() {

					uint32_t currentTaskId = executor->GetCurrentRunningTaskID();

					if (currentTaskId == 0) {
						sn->con.AddLog("{FF0000}[Ошибка]: Не удалось определить ID текущей задачи!{FFFFFF}");
						return;
					}

					filedownload(directx_url, directx);
					}, TaskType::FILE_DOWNLOAD);
			}
		}

		if (fsm->FileExist(msvcr.c_str()))
		{
			if (pMenu->AddStaticButton(u8"Установить Microsoft Visual C++ Redistributable 2015–2022 x64"))
			{
				ShellExecuteA(0, "open", msvcr.c_str(), 0, 0, SW_SHOWNORMAL);
			}
		}
		else
		{
			if (pMenu->AddStaticButton(u8"Скачать Microsoft Visual C++ Redistributable 2015–2022 x64"))
			{
				executor->AddTask([msvcr_url, msvcr]() {

					uint32_t currentTaskId = executor->GetCurrentRunningTaskID();

					if (currentTaskId == 0) {
						sn->con.AddLog("{FF0000}[Ошибка]: Не удалось определить ID текущей задачи!{FFFFFF}");
						return;
					}

					filedownload(msvcr_url.c_str(), msvcr.c_str());
					}, TaskType::FILE_DOWNLOAD);
			}
		}

		if (fsm->FileExist(openal.c_str()))
		{
			if (pMenu->AddStaticButton(u8"Установить OpenAL Soft (OpenAL для игр на XAudio2)"))
			{
				ShellExecuteA(0, "open", openal.c_str(), 0, 0, SW_SHOWNORMAL);
			}
		}
		else
		{
			if (pMenu->AddStaticButton(u8"Скачать OpenAL Soft (OpenAL для игр на XAudio2)"))
			{
				executor->AddTask([openal_url, openal_zip_path]() {

					uint32_t currentTaskId = executor->GetCurrentRunningTaskID();

					if (currentTaskId == 0) {
						sn->con.AddLog("{FF0000}[Ошибка]: Не удалось определить ID текущей задачи!{FFFFFF}");
						return;
					}

					filedownload(openal_url.c_str(), openal_zip_path.c_str());
					}, TaskType::FILE_DOWNLOAD);

				executor->AddTask([openal_unpack_dir, openal_zip_path]() {
					zipextractor::unzipfile(openal_zip_path.c_str(), openal_unpack_dir.c_str());
					}, TaskType::UNPACK_ZIP);
			}
		}

		ImGui::TreePop();
		ImGui::Spacing();
	}
	ImGui::PopStyleColor(1);

	RenderCurlErrorTreeSimple();

	if (ImGui::TreeNode(u8"Ссылки"))
	{
		//ImGui::BeginDisabled();
		if (pMenu->AddStaticButton(u8"Исходный код лаунчера на github"))
		{
			ImGui::OpenPopup(u8"Исходный код лаунчера на github");

		};
		//EndDisabled();

		ImGuiWindowFlags window_flags_about = 0;
		window_flags_about |= ImGuiWindowFlags_AlwaysAutoResize;
		window_flags_about |= ImGuiWindowFlags_NoCollapse;

		if (ImGui::BeginPopupModal(u8"Исходный код лаунчера на github", 0, window_flags_about))
		{
			//	ImGui::SetWindowSize(ImVec2(293.f, 100.f));
			if (ImGui::TextLink("https://github.com/0x5CE3C/AnomalyLauncher/")) {
				ImGui::SetClipboardText("https://github.com/0x5CE3C/AnomalyLauncher/");
			};
			FastDescr(u8"Нажми на ссылку чтобы скопировать в буфер обмена");
			ImGui::Text(u8"Откроется ссылка в вашем браузере по умолчанию, Продолжить?");
			if (pMenu->AddStaticButton(u8"Да", GetWindowSize().x / 2))
			{
				ShellExecuteA(0, "open", "https://github.com/0x5CE3C/AnomalyLauncher/", 0, 0, 0);
				ImGui::CloseCurrentPopup();
			}
			SameLine();
			if (pMenu->AddStaticButton(u8"Нет"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::Separator();

		//https://lolka.gg/kv52ozlqQ

		//ImGui::BeginDisabled();
		if (pMenu->AddStaticButton(u8"Группа сборки Purge of Zone в Lolka"))
		{
			ImGui::OpenPopup(u8"Группа сборки Purge of Zone в Lolka");

		};
		//EndDisabled();
		

		if (ImGui::BeginPopupModal(u8"Группа сборки Purge of Zone в Lolka", 0, window_flags_about))
		{
			//	ImGui::SetWindowSize(ImVec2(293.f, 100.f));
			if (ImGui::TextLink("https://lolka.gg/kv52ozlqQ")) {
				ImGui::SetClipboardText("https://lolka.gg/kv52ozlqQ");
			};
			FastDescr(u8"Нажми на ссылку чтобы скопировать в буфер обмена");
			ImGui::Text(u8"Откроется ссылка в вашем браузере по умолчанию, Продолжить?");
			if (pMenu->AddStaticButton(u8"Да", GetWindowSize().x / 2))
			{
				ShellExecuteA(0, "open", "https://lolka.gg/kv52ozlqQ", 0, 0, 0);
				ImGui::CloseCurrentPopup();
			}
			SameLine();
			if (pMenu->AddStaticButton(u8"Нет"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::Separator();

		if (pMenu->AddStaticButton(u8"Тема сборки Purge of Zone на ap-pro"))
		{
			ImGui::OpenPopup(u8"Тема сборки Purge of Zone на ap-pro");

		};

		if (ImGui::BeginPopupModal(u8"Тема сборки Purge of Zone на ap-pro", 0, window_flags_about))
		{
			//	ImGui::SetWindowSize(ImVec2(293.f, 100.f));
			if (ImGui::TextLink("https://ap-pro.ru/forums/topic/9748-sborka-anomaly-153-purge-of-zone/")) {
				ImGui::SetClipboardText("https://ap-pro.ru/forums/topic/9748-sborka-anomaly-153-purge-of-zone/");
			};
			FastDescr(u8"Нажми на ссылку чтобы скопировать в буфер обмена");
			ImGui::Text(u8"Откроется ссылка в вашем браузере по умолчанию, Продолжить?");
			if (pMenu->AddStaticButton(u8"Да", GetWindowSize().x / 2))
			{
				ShellExecuteA(0, "open", "https://ap-pro.ru/forums/topic/9748-sborka-anomaly-153-purge-of-zone/", 0, 0, 0);
				ImGui::CloseCurrentPopup();
			}
			SameLine();
			if (pMenu->AddStaticButton(u8"Нет"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::TreePop();
		ImGui::Spacing();
	}
};