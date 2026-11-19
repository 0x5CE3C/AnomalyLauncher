#pragma once


namespace ImFuncs
{
	void HelpDescr(const char* desc);
	void HelpDescr(const char* desc, ImColor Color);
	void FastDescr(const char* desc, ImColor Color);
	void FastDescr(const char* desc);
	void LineVertical();

	void GlitchText(const char* text);
	void GlitchText2(const char* text);
	void GlitchText3(const char* text);
	std::string animateString(const char* word, int rotationDelay);


    char getRandomSymbol(char from, char to);


    // В заголовке (или в классе лаунчера)
    struct TypewriterAnimator {
        std::string text;
        int currentIndex = 0;
        int framesPerStep = 3;      // шаг анимации: 1 символ каждые N кадров
        int frameCounter = 0;      // счётчик кадров

        TypewriterAnimator(const char* t, int step) : text(t), framesPerStep(step) {}

        void reset() {
            currentIndex = 0;
            frameCounter = 0;
        }

        // Вызывать один раз за кадр (перед отрисовкой ImGui)
        void update() {
            ++frameCounter;
            if (frameCounter >= framesPerStep) {
                frameCounter = 0;
                if (currentIndex < static_cast<int>(text.size())) {
                    ++currentIndex;
                }
            }
        }

        // Возвращает строку для отрисовки (без аллокации каждый раз, если использовать буфер)
        std::string getCurrent() const {
            if (currentIndex >= static_cast<int>(text.size())) {
                return text;
            }

            static char buffer[256];
            int len = static_cast<int>(text.size());
            for (int i = 0; i < len; ++i) {
                if (i < currentIndex) {
                    buffer[i] = text[i];
                }
                else {
                    buffer[i] = ImFuncs::getRandomSymbol(32, 126);
                }
            }
            buffer[len] = '\0';
            return std::string(buffer, len);
        }
    };
	
}

#ifndef EXTRA_IMGUI_H
#define EXTRA_IMGUI_H

namespace ImGui {

	void TextWithColors(const char* fmt, ...);	
}

#endif