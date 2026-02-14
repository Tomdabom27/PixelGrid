#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>

static inline sf::Color hsvToRgb(float hue_, float saturation, float value) {
    float hue = std::fmod(hue_, 360.0f);
    if (hue < 0.0f) hue += 360.0f;

    float c  = value * saturation;
    float hp = hue / 60.0f;
    float x  = c * (1.0f - std::fabs(std::fmod(hp, 2.0f) - 1.0f));
    float m  = value - c;

    float r = 0.0f, g = 0.0f, b = 0.0f;
    int sector = static_cast<int>(hp);
    switch (sector) {
        case 0: r = c; g = x; b = 0; break;
        case 1: r = x; g = c; b = 0; break;
        case 2: r = 0; g = c; b = x; break;
        case 3: r = 0; g = x; b = c; break;
        case 4: r = x; g = 0; b = c; break;
        default: r = c; g = 0; b = x; break;
    }

    unsigned char R = static_cast<unsigned char>(std::clamp((r + m) * 255.0f, 0.0f, 255.0f));
    unsigned char G = static_cast<unsigned char>(std::clamp((g + m) * 255.0f, 0.0f, 255.0f));
    unsigned char B = static_cast<unsigned char>(std::clamp((b + m) * 255.0f, 0.0f, 255.0f));
    return sf::Color(R, G, B, 255u);
}

int main() {
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    unsigned int windowWidthU  = desktopMode.size.x;
    unsigned int windowHeightU = desktopMode.size.y;
    int windowWidth  = static_cast<int>(windowWidthU);
    int windowHeight = static_cast<int>(windowHeightU);

    sf::RenderWindow window(desktopMode, "Scratch Pixel Grid Effect", sf::State::Fullscreen);
    window.setFramerateLimit(120);

    constexpr int CELL_SIZE = 2;
    const float CELL_SIZE_F = static_cast<float>(CELL_SIZE);

    const int cols = (windowWidth + CELL_SIZE - 1) / CELL_SIZE;
    const int rows = (windowHeight + CELL_SIZE - 1) / CELL_SIZE;

    const float centerX = windowWidth * 0.5f;
    const float centerY = windowHeight * 0.5f;

    std::vector<float> scratchX(cols);
    std::vector<float> scratchY(rows);
    std::vector<float> screenX(cols);
    std::vector<float> screenY(rows);

    const float startX = -centerX + (CELL_SIZE_F / 2.0f);
    const float startY =  centerY - (CELL_SIZE_F / 2.0f);

    for (int c = 0; c < cols; ++c) {
        float sx = startX + (c * CELL_SIZE_F);
        scratchX[c] = sx;
        screenX[c]  = centerX + sx;
    }
    for (int r = 0; r < rows; ++r) {
        float sy = startY - (r * CELL_SIZE_F);
        scratchY[r] = sy;
        screenY[r]  = centerY - sy;
    }

    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(windowWidth) * windowHeight * 4u);

    sf::Texture texture(sf::Vector2u(windowWidthU, windowHeightU));
    texture.setSmooth(false);
    sf::Sprite sprite(texture);

    std::vector<float> dx2(cols);
    std::vector<float> dy2(rows);

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (event->is<sf::Event::KeyPressed>()) {
                if (event->getIf<sf::Event::KeyPressed>()) {
                    auto kp = event->getIf<sf::Event::KeyPressed>();
                    if (kp->code == sf::Keyboard::Key::Escape) {
                        window.close();
                    }
                }
            }
        }

        sf::Vector2i mousePixelPos = sf::Mouse::getPosition(window);
        const float mouseX = static_cast<float>(mousePixelPos.x);
        const float mouseY = static_cast<float>(mousePixelPos.y);

        for (int c = 0; c < cols; ++c) {
            float dx = mouseX - screenX[c];
            dx2[c] = dx * dx;
        }
        for (int r = 0; r < rows; ++r) {
            float dy = mouseY - screenY[r];
            dy2[r] = dy * dy;
        }

        for (int r = 0; r < rows; ++r) {
            const int y0 = r * CELL_SIZE;
            if (y0 >= windowHeight) continue;
            for (int c = 0; c < cols; ++c) {
                const int x0 = c * CELL_SIZE;
                if (x0 >= windowWidth) continue;

                float sq = dx2[c] + dy2[r];
                if (sq < 1.0f) sq = 1.0f;

                float logDist = 0.5f * std::log(sq);

                float colorValue = (logDist * 200.0f + (scratchX[c] + scratchY[r]));

                sf::Color pxColor = hsvToRgb(colorValue, 1.0f, 1.0f);

                int blockW = std::min(CELL_SIZE, windowWidth - x0);
                int blockH = std::min(CELL_SIZE, windowHeight - y0);

                if (blockW == 2 && blockH == 2) {
                    std::size_t baseIndex0 = (static_cast<std::size_t>(y0) * windowWidth + static_cast<std::size_t>(x0)) * 4u;
                    std::size_t baseIndex1 = (static_cast<std::size_t>(y0 + 1) * windowWidth + static_cast<std::size_t>(x0)) * 4u;

                    pixels[baseIndex0 + 0] = pxColor.r;
                    pixels[baseIndex0 + 1] = pxColor.g;
                    pixels[baseIndex0 + 2] = pxColor.b;
                    pixels[baseIndex0 + 3] = pxColor.a;

                    pixels[baseIndex0 + 4] = pxColor.r;
                    pixels[baseIndex0 + 5] = pxColor.g;
                    pixels[baseIndex0 + 6] = pxColor.b;
                    pixels[baseIndex0 + 7] = pxColor.a;

                    pixels[baseIndex1 + 0] = pxColor.r;
                    pixels[baseIndex1 + 1] = pxColor.g;
                    pixels[baseIndex1 + 2] = pxColor.b;
                    pixels[baseIndex1 + 3] = pxColor.a;

                    pixels[baseIndex1 + 4] = pxColor.r;
                    pixels[baseIndex1 + 5] = pxColor.g;
                    pixels[baseIndex1 + 6] = pxColor.b;
                    pixels[baseIndex1 + 7] = pxColor.a;
                } else {
                    for (int by = 0; by < blockH; ++by) {
                        std::size_t rowIndex = (static_cast<std::size_t>(y0 + by) * windowWidth + static_cast<std::size_t>(x0)) * 4u;
                        for (int bx = 0; bx < blockW; ++bx) {
                            std::size_t idx = rowIndex + static_cast<std::size_t>(bx) * 4u;
                            pixels[idx + 0] = pxColor.r;
                            pixels[idx + 1] = pxColor.g;
                            pixels[idx + 2] = pxColor.b;
                            pixels[idx + 3] = pxColor.a;
                        }
                    }
                }
            }
        }

        texture.update(pixels.data());
        window.clear(sf::Color::Black);
        window.draw(sprite);
        window.display();
    }

    return 0;
}