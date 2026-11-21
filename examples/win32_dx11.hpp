#pragma once

#include <memory>
#include <string>

class Window {
public:
    Window(int width, int height, bool capture);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Valid() const;
    bool Poll();
    float DpiScale() const;
    void BeginFrame(bool capture, float deltaTime);
    bool Render(const std::string& capturePath = {});

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
