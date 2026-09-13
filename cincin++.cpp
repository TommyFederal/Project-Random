#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <optional>
#include <iostream>

// config
const unsigned int WINDOW_W = 950;
const unsigned int WINDOW_H = 550;
const unsigned int CHAR_SIZE = 26;
const float BOX_PADDING = 26.f;
const float BOX_WIDTH = 600.f;
const float LINE_SPACING = 34.f;
const float MARGIN = 24.f; // dekatnya kotakan dengan target posisi

// Alur penyesuaian Waktu Lirik
const float WORD_INTERVAL = 0.30f;   // Detik setiap kata2 muncul
const float POP_DURATION = 0.45f;    // Detik buat box ke posisinya
const float CLOSE_DURATION = 0.10f;   // Detik buat box sebelumnya agar ilang(kalo lu paham maksudnya)
const float HOLD_DURATION = 0.12f;    // detik dasar buat kalimat bertahan sebelum ilang

const float DRIFT_SPEED_MIN = 70.f;   // px/sec, closing box's ambient drift
const float DRIFT_SPEED_MAX = 140.f;
const float KNOCKBACK_SPEED = 220.f;  // px/sec, applied on crash

// Per-bait nahan override dalam detik(Seberapa lama kalimatnya bertahan sebelum nutup
// Gunakan -1.f buat balik ke HOLD_DURATION. Sama panjang buat "liriknya" dibawah
// isi selagi lu nyesuaiin waktunya
std::vector<float> lineHoldOverride = {
    4.19f, 3.06f, 3.49f, 2.01f, 4.47f, 3.08f, 4.69f, 3.33f
};

const std::vector<std::string> FONT_TYPE = {
    "font.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/Library/Fonts/Arial.ttf",
    "C:\\Windows\\Fonts\\arial.ttf"
};

std::vector<std::string> liriknya = {
    "Persetan kata siapa, mau bilang apa tak guna",
    "Mereka hanya tahu namamu, mereka takkan jadi diriku",
    "Persetan aturan cinta, tak tertulis di atas batu",
    "Apa kau ingin jadi benar, atau ingin jadi muda",
    "Semoga hidup kita trus, Begini-gini Saja",
    "Walau sungai menguap dan kurs tak masuk logika",
    "Semoga kita mencintai apa adanya",
    "Walau katanya sekarang ku bisa masuk penjara"
};

// Text wrapping
std::vector<std::string> wrapText(const std::string& text, const sf::Font& font, unsigned int charSize, float maxWidth) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word, currentLine;

    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        sf::Text testText(font, testLine, charSize);
        if (testText.getLocalBounds().size.x > maxWidth && !currentLine.empty()) {
            lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);
    if (lines.empty()) lines.push_back("");
    return lines;
}

// Returns, buat tiap hurup/kata2, characternya count
// intinya bikin kata2nya muncul satu persatu.
std::vector<size_t> computeWordBoundaries(const std::string& text) {
    std::vector<size_t> boundaries;
    size_t i = 0;
    while (i < text.size()) {
        size_t j = i;
        while (j < text.size() && text[j] != ' ' && text[j] != '\n') j++;
        while (j < text.size() && (text[j] == ' ' || text[j] == '\n')) j++;
        boundaries.push_back(j);
        i = j;
    }
    if (boundaries.empty() || boundaries.back() != text.size()) boundaries.push_back(text.size());
    return boundaries;
}

// Kotakan lirik
struct LyricBox {
    std::string rawText;
    std::vector<std::string> wrappedLines;
    std::string fullText;
    std::vector<size_t> wordBoundaries;
    size_t wordIndex = 0;
    float boxHeight = 0.f;
    size_t revealedChars = 0;
    float typeTimer = 0.f;
    float animTimer = 0.f;
    sf::Vector2f pos;
    sf::Vector2f startPos;
    sf::Vector2f targetPos;
    sf::Vector2f driftVel{0.f, 0.f};
    enum LocalState { POPPING_IN, TYPING, HOLD, CLOSING } state = POPPING_IN;
};

// Judul, lirik dan akhiran credit
enum AppState { TITLE, LYRICS, CREDITS };

LyricBox makeLyricBox(size_t index, const sf::Font& font) {
    LyricBox b;
    b.rawText = liriknya[index];
    b.wrappedLines = wrapText(b.rawText, font, CHAR_SIZE, BOX_WIDTH - 2 * BOX_PADDING);
    for (size_t i = 0; i < b.wrappedLines.size(); ++i) {
        b.fullText += b.wrappedLines[i];
        if (i + 1 < b.wrappedLines.size()) b.fullText += "\n";
    }
    b.boxHeight = b.wrappedLines.size() * LINE_SPACING + 2 * BOX_PADDING;
    b.wordBoundaries = computeWordBoundaries(b.fullText);

    float maxX = static_cast<float>(WINDOW_W) - BOX_WIDTH - MARGIN;
    float maxY = static_cast<float>(WINDOW_H) - b.boxHeight - MARGIN;
    if (maxX < MARGIN) maxX = MARGIN;
    if (maxY < MARGIN) maxY = MARGIN;
    float tx = MARGIN + (static_cast<float>(std::rand()) / RAND_MAX) * (maxX - MARGIN);
    float ty = MARGIN + (static_cast<float>(std::rand()) / RAND_MAX) * (maxY - MARGIN);
    b.targetPos = {tx, ty};

    float angle = static_cast<float>(std::rand() % 360) * 3.14159265f / 180.f;
    float dist = 350.f + static_cast<float>(std::rand() % 200);
    b.startPos = {tx + std::cos(angle) * dist, ty + std::sin(angle) * dist};
    b.pos = b.startPos;

    b.state = LyricBox::POPPING_IN;
    b.animTimer = 0.f;
    return b;
}

void drawLyricBox(sf::RenderWindow& window, const sf::Font& font, const LyricBox& box, float alphaF) {
    std::uint8_t alpha = static_cast<std::uint8_t>(alphaF < 0.f ? 0.f : (alphaF > 255.f ? 255.f : alphaF));

    sf::RectangleShape shadow(sf::Vector2f(BOX_WIDTH, box.boxHeight));
    shadow.setPosition({box.pos.x + 6, box.pos.y + 8});
    shadow.setFillColor(sf::Color(0, 0, 0, static_cast<std::uint8_t>(120.f * (alpha / 255.f))));
    window.draw(shadow);

    sf::RectangleShape rect(sf::Vector2f(BOX_WIDTH, box.boxHeight));
    rect.setPosition(box.pos);
    rect.setFillColor(sf::Color(35, 35, 48, alpha));
    rect.setOutlineThickness(3.f);
    rect.setOutlineColor(sf::Color(120, 170, 255, alpha));
    window.draw(rect);

    std::string shown = box.fullText.substr(0, box.revealedChars);
    sf::Text text(font, shown, CHAR_SIZE);
    text.setFillColor(sf::Color(240, 240, 245, alpha));
    text.setPosition({box.pos.x + BOX_PADDING, box.pos.y + BOX_PADDING});
    window.draw(text);
}

// Partikel, buat efek tabrakan
struct IconParticle {
    enum Type { HEART, RING, FACE, SPARKLE } type;
    sf::Vector2f pos;
    sf::Vector2f vel;
    float age = 0.f;
    float maxLife = 1.1f;
    float size = 20.f;
    std::string emoticon;
    sf::Color color;
};

void drawHeart(sf::RenderWindow& window, sf::Vector2f center, float size, sf::Color color) {
    float r = size * 0.28f;
    sf::CircleShape lobeL(r);
    lobeL.setOrigin({r, r});
    lobeL.setPosition({center.x - r * 0.6f, center.y - r * 0.4f});
    lobeL.setFillColor(color);
    window.draw(lobeL);

    sf::CircleShape lobeR(r);
    lobeR.setOrigin({r, r});
    lobeR.setPosition({center.x + r * 0.6f, center.y - r * 0.4f});
    lobeR.setFillColor(color);
    window.draw(lobeR);

    sf::ConvexShape tri(3);
    tri.setPoint(0, {center.x - r * 1.6f, center.y - r * 0.1f});
    tri.setPoint(1, {center.x + r * 1.6f, center.y - r * 0.1f});
    tri.setPoint(2, {center.x, center.y + r * 1.9f});
    tri.setFillColor(color);
    window.draw(tri);
}

void drawRing(sf::RenderWindow& window, sf::Vector2f center, float radius, sf::Color color) {
    sf::CircleShape ring(radius);
    ring.setOrigin({radius, radius});
    ring.setPosition(center);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineThickness(radius * 0.35f);
    ring.setOutlineColor(color);
    window.draw(ring);
}

void drawFace(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f center,
              const std::string& emoticon, sf::Color color, unsigned int charSize) {
    sf::Text t(font, emoticon, charSize);
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    t.setPosition(center);
    t.setFillColor(color);
    window.draw(t);
}

void drawSparkle(sf::RenderWindow& window, sf::Vector2f center, float size, sf::Color color) {
    sf::RectangleShape barH(sf::Vector2f(size, size * 0.18f));
    barH.setOrigin({size / 2.f, size * 0.09f});
    barH.setPosition(center);
    barH.setFillColor(color);
    window.draw(barH);

    sf::RectangleShape barV(sf::Vector2f(size * 0.18f, size));
    barV.setOrigin({size * 0.09f, size / 2.f});
    barV.setPosition(center);
    barV.setFillColor(color);
    window.draw(barV);
}

IconParticle::Type pickIconType(const std::string& text) {
    std::string lower = text;
    for (auto& ch : lower) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    if (lower.find("cinta") != std::string::npos) return IconParticle::HEART;
    if (lower.find('?') != std::string::npos) return IconParticle::FACE;
    return IconParticle::SPARKLE;
}

void spawnBurst(std::vector<IconParticle>& particles, sf::Vector2f center, IconParticle::Type keywordType) {
    // Partikel Cincin
    for (int i = 0; i < 2; ++i) {
        float angle = static_cast<float>(std::rand() % 360) * 3.14159265f / 180.f;
        float speed = 60.f + static_cast<float>(std::rand() % 60);
        IconParticle p;
        p.type = IconParticle::RING;
        p.pos = center;
        p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.maxLife = 1.0f + static_cast<float>(std::rand() % 40) / 100.f;
        p.size = 16.f + static_cast<float>(std::rand() % 10);
        p.color = sf::Color(255, 215, 120);
        particles.push_back(p);
    }
    // Plus a handful matching this line's mood.
    for (int i = 0; i < 5; ++i) {
        float angle = static_cast<float>(std::rand() % 360) * 3.14159265f / 180.f;
        float speed = 70.f + static_cast<float>(std::rand() % 90);
        IconParticle p;
        p.type = keywordType;
        p.pos = center;
        p.vel = {std::cos(angle) * speed, std::sin(angle) * speed};
        p.maxLife = 0.9f + static_cast<float>(std::rand() % 50) / 100.f;
        p.size = 18.f + static_cast<float>(std::rand() % 14);
        if (keywordType == IconParticle::HEART) {
            p.color = sf::Color(255, 120, 150);
        } else if (keywordType == IconParticle::FACE) {
            static const std::vector<std::string> faces = {":)", "^_^", "<3", ":D", "o.O"};
            p.emoticon = faces[std::rand() % faces.size()];
            p.color = sf::Color(230, 230, 245);
        } else {
            p.color = sf::Color(200, 220, 255);
        }
        particles.push_back(p);
    }
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    sf::Font font;
    bool loaded = false;
    for (const auto& path : FONT_TYPE) {
        if (font.openFromFile(path)) { loaded = true; break; }
    }
    if (!loaded) {
        std::cerr << "Gak ada ini fontnya cok, masukin font.ttf deket executable, ato edit FONT_TYPE di source.\n";
        return 1;
    }

    sf::RenderWindow window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Cincin - Hindia", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    size_t lyricIndex = 0;
    bool currentActive = true;
    bool previousActive = false;
    bool finished = false;
    bool crashTriggered = false;

    LyricBox current = makeLyricBox(lyricIndex, font);
    LyricBox previous;
    AppState appState = TITLE;
    
    std::vector<IconParticle> particles;

    sf::Clock clock;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape)
                    window.close();
                if (keyPressed->code == sf::Keyboard::Key::Enter && appState == TITLE)
                    appState = LYRICS;
            }
        }

        float dt = clock.restart().asSeconds();

        if (appState == LYRICS) {
            if (previousActive) {
            previous.animTimer += dt;
            previous.pos.x += previous.driftVel.x * dt;
            previous.pos.y += previous.driftVel.y * dt;
            if (previous.animTimer >= CLOSE_DURATION) previousActive = false;
        }

        // Current box: fly in -> type word-by-word -> hold -> hand off -> jadi deh
        if (currentActive) {
            if (current.state == LyricBox::POPPING_IN) {
                current.animTimer += dt;
                float t = current.animTimer / POP_DURATION;
                if (t > 1.f) t = 1.f;
                float eased = 1.f - (1.f - t) * (1.f - t);
                current.pos.x = current.startPos.x + (current.targetPos.x - current.startPos.x) * eased;
                current.pos.y = current.startPos.y + (current.targetPos.y - current.startPos.y) * eased;
                if (t >= 1.f) {
                    current.state = LyricBox::TYPING;
                    current.typeTimer = 0.f;
                    current.revealedChars = 0;
                    current.wordIndex = 0;
                    current.pos = current.targetPos;
                }
            } else if (current.state == LyricBox::TYPING) {
                current.typeTimer += dt;
                while (current.typeTimer >= WORD_INTERVAL && current.wordIndex < current.wordBoundaries.size()) {
                    current.typeTimer -= WORD_INTERVAL;
                    current.revealedChars = current.wordBoundaries[current.wordIndex];
                    current.wordIndex++;
                }
                if (current.wordIndex >= current.wordBoundaries.size()) {
                    current.state = LyricBox::HOLD;
                    current.animTimer = 0.f;
                }
            } else if (current.state == LyricBox::HOLD) {
                current.animTimer += dt;
                float holdDur = (lyricIndex < lineHoldOverride.size() && lineHoldOverride[lyricIndex] >= 0.f)
                                    ? lineHoldOverride[lyricIndex] : HOLD_DURATION;
                if (current.animTimer >= holdDur) {
                    current.state = LyricBox::CLOSING;
                    current.animTimer = 0.f;
                    float angle = static_cast<float>(std::rand() % 360) * 3.14159265f / 180.f;
                    float speed = DRIFT_SPEED_MIN + static_cast<float>(std::rand() % static_cast<int>(DRIFT_SPEED_MAX - DRIFT_SPEED_MIN));
                    current.driftVel = {std::cos(angle) * speed, std::sin(angle) * speed};
                    previous = current;
                    previousActive = true;
                    crashTriggered = false;

                    lyricIndex++;
                    if (lyricIndex >= liriknya.size()) {
                        currentActive = false;
                    } else {
                        current = makeLyricBox(lyricIndex, font);
                        currentActive = true;
                    }
                }
            }
        }

        // Crash detection
        if (previousActive && currentActive && !crashTriggered) {
            sf::FloatRect rectPrev({previous.pos.x, previous.pos.y}, {BOX_WIDTH, previous.boxHeight});
            sf::FloatRect rectCurr({current.pos.x, current.pos.y}, {BOX_WIDTH, current.boxHeight});
            std::optional<sf::FloatRect> overlap = rectPrev.findIntersection(rectCurr);
            if (overlap.has_value()) {
                sf::Vector2f center = {
                    overlap->position.x + overlap->size.x / 2.f,
                    overlap->position.y + overlap->size.y / 2.f
                };
                spawnBurst(particles, center, pickIconType(current.rawText));

                sf::Vector2f prevCenter = {previous.pos.x + BOX_WIDTH / 2.f, previous.pos.y + previous.boxHeight / 2.f};
                sf::Vector2f currCenter = {current.pos.x + BOX_WIDTH / 2.f, current.pos.y + current.boxHeight / 2.f};
                sf::Vector2f pushDir = {prevCenter.x - currCenter.x, prevCenter.y - currCenter.y};
                float len = std::sqrt(pushDir.x * pushDir.x + pushDir.y * pushDir.y);
                if (len < 1.f) { pushDir = {1.f, 0.f}; len = 1.f; }
                pushDir.x /= len; pushDir.y /= len;
                previous.driftVel = {pushDir.x * KNOCKBACK_SPEED, pushDir.y * KNOCKBACK_SPEED};

                crashTriggered = true;
            }
        }

        if (!currentActive && !previousActive) {
            finished = true;
            appState = CREDITS;
        }
    }
        // Update Partikel
        for (auto& p : particles) {
            p.age += dt;
            p.pos.x += p.vel.x * dt;
            p.pos.y += p.vel.y * dt;
            p.vel.x *= 0.98f;
            p.vel.y *= 0.98f;
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(),
            [](const IconParticle& p) { return p.age >= p.maxLife; }), particles.end());

        // Draw
        window.clear(sf::Color(18, 18, 24));
        if (appState == TITLE) {
            sf::Text titleText(font, "CINCIN - HINDIA", CHAR_SIZE + 12); // judul
            sf::FloatRect tb = titleText.getLocalBounds();
            titleText.setPosition({(WINDOW_W - tb.size.x) / 2.f, WINDOW_H / 2.f - 40.f});
            titleText.setFillColor(sf::Color(240, 240, 245));
            window.draw(titleText);

            sf::Text hint(font, "Pencet ENTER untuk mulai", CHAR_SIZE - 8); // Subtitel kecil
            sf::FloatRect hb = hint.getLocalBounds();
            hint.setPosition({(WINDOW_W - hb.size.x) / 2.f, WINDOW_H / 2.f + 20.f});
            hint.setFillColor(sf::Color(150, 150, 160));
            window.draw(hint);
        }

        if (previousActive) {
            float t = previous.animTimer / CLOSE_DURATION;
            if (t > 1.f) t = 1.f;
            drawLyricBox(window, font, previous, 255.f * (1.f - t));
        }
        if (currentActive) {
            float alpha = 255.f;
            if (current.state == LyricBox::POPPING_IN) {
                float t = current.animTimer / POP_DURATION;
                if (t > 1.f) t = 1.f;
                alpha = 255.f * t;
            }
            drawLyricBox(window, font, current, alpha);
        }

        for (auto& p : particles) {
            float lifeFactor = 1.f - (p.age / p.maxLife);
            if (lifeFactor < 0.f) lifeFactor = 0.f;
            sf::Color c = p.color;
            c.a = static_cast<std::uint8_t>(255.f * lifeFactor);
            switch (p.type) {
                case IconParticle::HEART:   drawHeart(window, p.pos, p.size, c); break;
                case IconParticle::RING:    drawRing(window, p.pos, p.size * 0.5f, c); break;
                case IconParticle::FACE:    drawFace(window, font, p.pos, p.emoticon, c, static_cast<unsigned int>(p.size)); break;
                case IconParticle::SPARKLE: drawSparkle(window, p.pos, p.size, c); break;
            }
        }

        if (appState == CREDITS) {
            sf::Text creditsText(font, "Script oleh Tommy Federal :)", CHAR_SIZE + 4); // Ukuran
            sf::FloatRect cb = creditsText.getLocalBounds();
            creditsText.setPosition({(WINDOW_W - cb.size.x) / 2.f, (WINDOW_H - cb.size.y) / 2.f});
            creditsText.setFillColor(sf::Color(150, 150, 160));
            window.draw(creditsText);
        }

        window.display();
    }

    return 0;
}