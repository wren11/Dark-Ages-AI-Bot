/**
 * @file ui_manager.cpp
 * @brief Implementation of the in-game UI manager
 */
#include "ui_manager.h"
#include "../utils/logging.h"
#include <algorithm>
#include <Windows.h>
#include <memory>
#include <string>
#include "../utils/memory/memory_manager.h"
#include "../constants.h"

namespace ui {

// Global variables for rendering
HDC g_hDC = nullptr;                   // Device context for rendering
HFONT g_hFont = nullptr;               // Default font
HFONT g_hFontBold = nullptr;           // Bold font
HFONT g_hFontItalic = nullptr;         // Italic font
HFONT g_hFontBoldItalic = nullptr;     // Bold italic font
int g_mouseX = 0;                      // Current mouse X position
int g_mouseY = 0;                      // Current mouse Y position
bool g_mouseDown = false;              // Is mouse button down
UIElementPtr g_hoveredElement = nullptr; // Currently hovered element
UIElementPtr g_focusedElement = nullptr; // Currently focused element

// Helper functions for rendering
void DrawRect(const Rect& rect, const Color& color) {
    if (!g_hDC) return;
    
    HBRUSH brush = CreateSolidBrush(RGB(color.r, color.g, color.b));
    RECT rc = { rect.x, rect.y, rect.x + rect.width, rect.y + rect.height };
    
    // Adjust alpha (simple implementation - would need DirectX for proper alpha)
    int alpha = color.a;
    if (alpha < 255) {
        // We're simulating alpha by creating a slightly transparent brush
        BLENDFUNCTION blend = { AC_SRC_OVER, 0, static_cast<BYTE>(alpha), 0 };
        AlphaBlend(g_hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                   g_hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                   blend);
    } else {
        FillRect(g_hDC, &rc, brush);
    }
    
    DeleteObject(brush);
}

void DrawBorder(const Rect& rect, const Color& color, int thickness = 1) {
    if (!g_hDC) return;
    
    HPEN pen = CreatePen(PS_SOLID, thickness, RGB(color.r, color.g, color.b));
    HPEN oldPen = (HPEN)SelectObject(g_hDC, pen);
    
    RECT rc = { rect.x, rect.y, rect.x + rect.width, rect.y + rect.height };
    MoveToEx(g_hDC, rc.left, rc.top, nullptr);
    LineTo(g_hDC, rc.right - 1, rc.top);
    LineTo(g_hDC, rc.right - 1, rc.bottom - 1);
    LineTo(g_hDC, rc.left, rc.bottom - 1);
    LineTo(g_hDC, rc.left, rc.top);
    
    SelectObject(g_hDC, oldPen);
    DeleteObject(pen);
}

void DrawText(const std::string& text, const Rect& rect, const Color& color, FontStyle style = FontStyle::Normal, bool centered = true) {
    if (!g_hDC) return;
    
    HFONT hFont = nullptr;
    switch (style) {
        case FontStyle::Bold:
            hFont = g_hFontBold;
            break;
        case FontStyle::Italic:
            hFont = g_hFontItalic;
            break;
        case FontStyle::BoldItalic:
            hFont = g_hFontBoldItalic;
            break;
        default:
            hFont = g_hFont;
            break;
    }
    
    HFONT oldFont = (HFONT)SelectObject(g_hDC, hFont);
    SetTextColor(g_hDC, RGB(color.r, color.g, color.b));
    SetBkMode(g_hDC, TRANSPARENT);
    
    RECT rc = { rect.x, rect.y, rect.x + rect.width, rect.y + rect.height };
    UINT format = DT_SINGLELINE | DT_VCENTER;
    if (centered) {
        format |= DT_CENTER;
    } else {
        format |= DT_LEFT;
        rc.left += 5; // Add some padding
    }
    
    DrawTextA(g_hDC, text.c_str(), -1, &rc, format);
    
    SelectObject(g_hDC, oldFont);
}

void DrawCheckmark(const Rect& rect, const Color& color) {
    if (!g_hDC) return;
    
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(color.r, color.g, color.b));
    HPEN oldPen = (HPEN)SelectObject(g_hDC, pen);
    
    int left = rect.x + rect.width / 4;
    int middle = rect.y + rect.height / 2;
    int right = rect.x + rect.width * 3 / 4;
    int bottom = rect.y + rect.height * 3 / 4;
    
    MoveToEx(g_hDC, left, middle, nullptr);
    LineTo(g_hDC, rect.x + rect.width / 2, bottom);
    LineTo(g_hDC, right, rect.y + rect.height / 3);
    
    SelectObject(g_hDC, oldPen);
    DeleteObject(pen);
}

void DrawArrow(const Rect& rect, const Color& color, bool down) {
    if (!g_hDC) return;
    
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(color.r, color.g, color.b));
    HPEN oldPen = (HPEN)SelectObject(g_hDC, pen);
    HBRUSH brush = CreateSolidBrush(RGB(color.r, color.g, color.b));
    HBRUSH oldBrush = (HBRUSH)SelectObject(g_hDC, brush);
    
    POINT points[3];
    if (down) {
        points[0] = { rect.x + rect.width / 4, rect.y + rect.height / 3 };
        points[1] = { rect.x + rect.width * 3 / 4, rect.y + rect.height / 3 };
        points[2] = { rect.x + rect.width / 2, rect.y + rect.height * 2 / 3 };
    } else {
        points[0] = { rect.x + rect.width / 4, rect.y + rect.height * 2 / 3 };
        points[1] = { rect.x + rect.width * 3 / 4, rect.y + rect.height * 2 / 3 };
        points[2] = { rect.x + rect.width / 2, rect.y + rect.height / 3 };
    }
    
    Polygon(g_hDC, points, 3);
    
    SelectObject(g_hDC, oldPen);
    SelectObject(g_hDC, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

// UIManager implementation
UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

bool UIManager::initialize() {
    if (initialized_) return true;
    
    try {
        // Initialize the device context
        HWND hwnd = FindWindowA("Darkages", nullptr);
        if (!hwnd) {
            utils::Logging::error("Failed to find Dark Ages window");
            return false;
        }
        
        g_hDC = GetDC(hwnd);
        if (!g_hDC) {
            utils::Logging::error("Failed to get device context");
            return false;
        }
        
        // Create fonts
        g_hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        
        g_hFontBold = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        
        g_hFontItalic = CreateFontA(14, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        
        g_hFontBoldItalic = CreateFontA(14, 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                       DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        
        if (!g_hFont || !g_hFontBold || !g_hFontItalic || !g_hFontBoldItalic) {
            utils::Logging::error("Failed to create fonts");
            return false;
        }
        
        initialized_ = true;
        return true;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to initialize UI manager: " + std::string(e.what()));
        return false;
    }
}

void UIManager::cleanup() {
    if (!initialized_) return;
    
    try {
        // Delete fonts
        if (g_hFont) DeleteObject(g_hFont);
        if (g_hFontBold) DeleteObject(g_hFontBold);
        if (g_hFontItalic) DeleteObject(g_hFontItalic);
        if (g_hFontBoldItalic) DeleteObject(g_hFontBoldItalic);
        
        // Release device context
        if (g_hDC) {
            HWND hwnd = FindWindowA("Darkages", nullptr);
            if (hwnd) {
                ReleaseDC(hwnd, g_hDC);
            }
        }
        
        // Clear windows
        windows_.clear();
        
        g_hDC = nullptr;
        g_hFont = nullptr;
        g_hFontBold = nullptr;
        g_hFontItalic = nullptr;
        g_hFontBoldItalic = nullptr;
        g_hoveredElement = nullptr;
        g_focusedElement = nullptr;
        
        initialized_ = false;
    }
    catch (const std::exception& e) {
        utils::Logging::error("Failed to cleanup UI manager: " + std::string(e.what()));
    }
}

WindowPtr UIManager::createWindow(const std::string& title, int x, int y, int width, int height) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto window = std::make_shared<Window>(title, Rect(x, y, width, height));
    windows_.push_back(window);
    return window;
}

void UIManager::removeWindow(const WindowPtr& window) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(windows_.begin(), windows_.end(), window);
    if (it != windows_.end()) {
        windows_.erase(it);
    }
}

void UIManager::removeWindowById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(windows_.begin(), windows_.end(),
                         [&id](const WindowPtr& window) { return window->getId() == id; });
    if (it != windows_.end()) {
        windows_.erase(it);
    }
}

WindowPtr UIManager::getWindowById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find_if(windows_.begin(), windows_.end(),
                         [&id](const WindowPtr& window) { return window->getId() == id; });
    if (it != windows_.end()) {
        return *it;
    }
    
    return nullptr;
}

void UIManager::render() {
    if (!initialized_ || !visible_ || !g_hDC) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto it = windows_.rbegin(); it != windows_.rend(); ++it) {
        if ((*it)->isVisible()) {
            (*it)->render();
        }
    }
}

void UIManager::handleMouseMove(int x, int y) {
    if (!initialized_ || !visible_) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    g_mouseX = x;
    g_mouseY = y;
    
    UIElementPtr hoveredElement = findElementAt(x, y);
    
    if (g_hoveredElement != hoveredElement) {
        if (g_hoveredElement) {
            g_hoveredElement->handleHover(false, false);
        }
        
        g_hoveredElement = hoveredElement;
        
        if (g_hoveredElement) {
            g_hoveredElement->handleHover(x, y);
        }
    }
    else if (g_hoveredElement) {
        g_hoveredElement->handleHover(x, y);
    }
}

void UIManager::handleMouseClick(int x, int y) {
    if (!initialized_ || !visible_) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    g_mouseX = x;
    g_mouseY = y;
    g_mouseDown = true;
    
    // Reset focused element
    g_focusedElement = nullptr;
    
    // Find the element under the cursor
    UIElementPtr clickedElement = findElementAt(x, y);
    
    if (clickedElement) {
        g_focusedElement = clickedElement;
        clickedElement->handleClick(x, y);
    }
    
    g_mouseDown = false;
}

void UIManager::handleKeyPress(char key) {
    if (!initialized_ || !visible_) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (g_focusedElement) {
        g_focusedElement->handleKeyPress(key);
    }
}

UIElementPtr UIManager::findElementAt(int x, int y) {
    // First check if we clicked on a window
    for (auto it = windows_.begin(); it != windows_.end(); ++it) {
        auto window = *it;
        
        if (window->isVisible() && window->getBounds().contains(x, y)) {
            // Found a window, check if we clicked on an element in the window
            auto element = window->findElementAt(x, y);
            if (element) {
                return element;
            }
            
            // No element found, return the window
            return window;
        }
    }
    
    return nullptr;
}

// Window implementation
void Window::render() {
    if (!visible_) return;
    
    // Draw window background
    DrawRect(bounds_, backgroundColor_);
    
    // Draw window border
    DrawBorder(bounds_, borderColor_, 1);
    
    // Draw title bar
    Rect titleBarRect(bounds_.x, bounds_.y, bounds_.width, titleBarHeight_);
    DrawRect(titleBarRect, Color(backgroundColor_.r / 2, backgroundColor_.g / 2, backgroundColor_.b / 2, backgroundColor_.a));
    DrawText(title_, titleBarRect, titleColor_);
    
    // Draw elements
    for (auto it = elements_.begin(); it != elements_.end(); ++it) {
        if ((*it)->isVisible()) {
            (*it)->render();
        }
    }
}

bool Window::handleClick(int x, int y) {
    if (!visible_ || !bounds_.contains(x, y)) return false;
    
    // Check if clicking on title bar (for dragging)
    Rect titleBarRect(bounds_.x, bounds_.y, bounds_.width, titleBarHeight_);
    if (draggable_ && titleBarRect.contains(x, y)) {
        dragging_ = true;
        dragOffsetX_ = x - bounds_.x;
        dragOffsetY_ = y - bounds_.y;
        return true;
    }
    
    // Check if clicking on any element
    for (auto it = elements_.rbegin(); it != elements_.rend(); ++it) {
        if ((*it)->isVisible() && (*it)->getBounds().contains(x, y)) {
            return (*it)->handleClick(x, y);
        }
    }
    
    return true; // Click handled by window
}

bool Window::handleHover(int x, int y) {
    if (!visible_) return false;
    
    if (dragging_) {
        // Update window position
        bounds_.x = x - dragOffsetX_;
        bounds_.y = y - dragOffsetY_;
        
        // Check if mouse button is released
        if (!g_mouseDown) {
            dragging_ = false;
        }
        
        return true;
    }
    
    // Check if hovering over any element
    for (auto it = elements_.rbegin(); it != elements_.rend(); ++it) {
        if ((*it)->isVisible() && (*it)->getBounds().contains(x, y)) {
            return (*it)->handleHover(x, y);
        }
    }
    
    return bounds_.contains(x, y);
}

void Window::addElement(const UIElementPtr& element) {
    elements_.push_back(element);
}

void Window::removeElement(const UIElementPtr& element) {
    auto it = std::find(elements_.begin(), elements_.end(), element);
    if (it != elements_.end()) {
        elements_.erase(it);
    }
}

void Window::removeElementById(const std::string& id) {
    auto it = std::find_if(elements_.begin(), elements_.end(),
                         [&id](const UIElementPtr& element) { return element->getId() == id; });
    if (it != elements_.end()) {
        elements_.erase(it);
    }
}

UIElementPtr Window::getElementById(const std::string& id) {
    auto it = std::find_if(elements_.begin(), elements_.end(),
                         [&id](const UIElementPtr& element) { return element->getId() == id; });
    if (it != elements_.end()) {
        return *it;
    }
    
    // Check child elements recursively
    for (auto& element : elements_) {
        auto window = std::dynamic_pointer_cast<Window>(element);
        if (window) {
            auto childElement = window->getElementById(id);
            if (childElement) {
                return childElement;
            }
        }
    }
    
    return nullptr;
}

// Button implementation
void Button::render() {
    if (!visible_) return;
    
    // Draw button background
    Color bgColor = isHovering_ ? hoverColor_ : backgroundColor_;
    DrawRect(bounds_, bgColor);
    
    // Draw button border
    DrawBorder(bounds_, borderColor_);
    
    // Draw button text
    DrawText(text_, bounds_, textColor_);
}

bool Button::handleClick(int x, int y) {
    if (!visible_ || !enabled_ || !bounds_.contains(x, y)) return false;
    
    if (onClick_) {
        onClick_();
    }
    
    return true;
}

bool Button::handleHover(int x, int y) {
    if (!visible_ || !enabled_) return false;
    
    bool wasHovering = isHovering_;
    isHovering_ = bounds_.contains(x, y);
    
    return isHovering_ != wasHovering;
}

// Label implementation
void Label::render() {
    if (!visible_) return;
    
    // Draw background if not transparent
    if (backgroundColor_.a > 0) {
        DrawRect(bounds_, backgroundColor_);
    }
    
    // Draw text
    DrawText(text_, bounds_, textColor_, fontStyle_, false);
}

// CheckBox implementation
void CheckBox::render() {
    if (!visible_) return;
    
    // Draw checkbox
    Rect checkRect(bounds_.x, bounds_.y + (bounds_.height - boxSize_) / 2, boxSize_, boxSize_);
    DrawRect(checkRect, boxColor_);
    DrawBorder(checkRect, Color(200, 200, 200));
    
    // Draw checkmark if checked
    if (checked_) {
        DrawCheckmark(checkRect, checkColor_);
    }
    
    // Draw label
    Rect textRect(bounds_.x + boxSize_ + 5, bounds_.y, bounds_.width - boxSize_ - 5, bounds_.height);
    DrawText(text_, textRect, textColor_, FontStyle::Normal, false);
}

bool CheckBox::handleClick(int x, int y) {
    if (!visible_ || !enabled_ || !bounds_.contains(x, y)) return false;
    
    checked_ = !checked_;
    
    if (onChange_) {
        onChange_(checked_);
    }
    
    return true;
}

// Menu implementation
void Menu::render() {
    if (!visible_) return;
    
    // Draw menu background
    DrawRect(bounds_, backgroundColor_);
    
    // Draw menu border
    DrawBorder(bounds_, borderColor_);
    
    // Calculate text area (leave space for arrow)
    Rect textRect(bounds_.x, bounds_.y, bounds_.width - 20, bounds_.height);
    
    // Draw selected item text or placeholder
    std::string displayText = (selectedIndex_ >= 0 && selectedIndex_ < items_.size()) ? 
                            items_[selectedIndex_] : "Select...";
    
    DrawText(displayText, textRect, textColor_, FontStyle::Normal, false);
    
    // Draw dropdown arrow
    Rect arrowRect(bounds_.x + bounds_.width - 20, bounds_.y, 20, bounds_.height);
    DrawArrow(arrowRect, textColor_, !expanded_);
    
    // Draw dropdown list if expanded
    if (expanded_) {
        int totalHeight = itemHeight_ * items_.size();
        Rect dropdownRect(bounds_.x, bounds_.y + bounds_.height, bounds_.width, totalHeight);
        DrawRect(dropdownRect, backgroundColor_);
        DrawBorder(dropdownRect, borderColor_);
        
        for (size_t i = 0; i < items_.size(); i++) {
            Rect itemRect(bounds_.x, bounds_.y + bounds_.height + i * itemHeight_, 
                         bounds_.width, itemHeight_);
            
            // Highlight hovered item
            if ((int)i == hoverIndex_) {
                DrawRect(itemRect, hoverColor_);
            }
            
            // Highlight selected item
            if ((int)i == selectedIndex_) {
                DrawRect(Rect(itemRect.x + 2, itemRect.y + 2, 4, itemRect.height - 4), 
                        Color(255, 255, 255));
            }
            
            // Draw item text
            DrawText(items_[i], itemRect, textColor_, FontStyle::Normal, false);
        }
    }
}

bool Menu::handleClick(int x, int y) {
    if (!visible_ || !enabled_) return false;
    
    // Check if clicked on main area
    if (bounds_.contains(x, y)) {
        expanded_ = !expanded_;
        return true;
    }
    
    // Check if clicked on dropdown area
    if (expanded_) {
        int totalHeight = itemHeight_ * items_.size();
        Rect dropdownRect(bounds_.x, bounds_.y + bounds_.height, bounds_.width, totalHeight);
        
        if (dropdownRect.contains(x, y)) {
            int itemIndex = (y - (bounds_.y + bounds_.height)) / itemHeight_;
            
            if (itemIndex >= 0 && itemIndex < static_cast<int>(items_.size())) {
                if (selectedIndex_ != itemIndex) {
                    selectedIndex_ = itemIndex;
                    
                    if (onChange_) {
                        onChange_(selectedIndex_);
                    }
                }
                
                expanded_ = false;
                return true;
            }
        }
        else {
            // Click outside the dropdown area, close it
            expanded_ = false;
            return true;
        }
    }
    
    return false;
}

bool Menu::handleHover(int x, int y) {
    if (!visible_ || !enabled_) return false;
    
    if (expanded_) {
        int totalHeight = itemHeight_ * items_.size();
        Rect dropdownRect(bounds_.x, bounds_.y + bounds_.height, bounds_.width, totalHeight);
        
        if (dropdownRect.contains(x, y)) {
            int newHoverIndex = (y - (bounds_.y + bounds_.height)) / itemHeight_;
            
            if (newHoverIndex != hoverIndex_ && newHoverIndex >= 0 && 
                newHoverIndex < static_cast<int>(items_.size())) {
                hoverIndex_ = newHoverIndex;
                return true;
            }
            
            return false;
        }
    }
    
    if (hoverIndex_ != -1) {
        hoverIndex_ = -1;
        return true;
    }
    
    return false;
}

} // namespace ui
