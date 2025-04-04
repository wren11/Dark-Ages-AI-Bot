/**
 * @file ui_manager.h
 * @brief Manages in-game UI elements and rendering
 */
#pragma once

#include <Windows.h>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "../utils/logging.h"
#include "../constants.h"

namespace ui {

// Forward declarations
class UIElement;
class Window;
class Button;
class Label;
class CheckBox;
class Slider;
class TextBox;
class Menu;

using UIElementPtr = std::shared_ptr<UIElement>;
using WindowPtr = std::shared_ptr<Window>;
using ButtonPtr = std::shared_ptr<Button>;
using LabelPtr = std::shared_ptr<Label>;
using CheckBoxPtr = std::shared_ptr<CheckBox>;
using SliderPtr = std::shared_ptr<Slider>;
using TextBoxPtr = std::shared_ptr<TextBox>;
using MenuPtr = std::shared_ptr<Menu>;

/**
 * @brief Color structure for UI elements
 */
struct Color {
    uint8_t r, g, b, a;
    
    Color(uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
    
    static Color Black() { return Color(0, 0, 0, 255); }
    static Color White() { return Color(255, 255, 255, 255); }
    static Color Red() { return Color(255, 0, 0, 255); }
    static Color Green() { return Color(0, 255, 0, 255); }
    static Color Blue() { return Color(0, 0, 255, 255); }
    static Color Yellow() { return Color(255, 255, 0, 255); }
    static Color Cyan() { return Color(0, 255, 255, 255); }
    static Color Magenta() { return Color(255, 0, 255, 255); }
    static Color Transparent() { return Color(0, 0, 0, 0); }
};

/**
 * @brief Rectangle structure for UI positioning
 */
struct Rect {
    int x, y, width, height;
    
    Rect(int x = 0, int y = 0, int width = 0, int height = 0)
        : x(x), y(y), width(width), height(height) {}
    
    bool contains(int px, int py) const {
        return px >= x && px < x + width && py >= y && py < y + height;
    }
    
    bool intersects(const Rect& other) const {
        return !(x + width <= other.x || other.x + other.width <= x ||
                y + height <= other.y || other.y + other.height <= y);
    }
};

/**
 * @brief Font style for text rendering
 */
enum class FontStyle {
    Normal,
    Bold,
    Italic,
    BoldItalic
};

/**
 * @brief Base class for all UI elements
 */
class UIElement {
public:
    UIElement(const Rect& bounds = Rect())
        : bounds_(bounds), visible_(true), enabled_(true), id_("") {}
    
    virtual ~UIElement() = default;
    
    // Core methods
    virtual void render() = 0;
    virtual bool handleClick(int x, int y) { return false; }
    virtual bool handleHover(int x, int y) { return false; }
    virtual bool handleKeyPress(char key) { return false; }
    
    // Property getters/setters
    void setPosition(int x, int y) { 
        bounds_.x = x; 
        bounds_.y = y; 
    }
    
    void setSize(int width, int height) { 
        bounds_.width = width; 
        bounds_.height = height; 
    }
    
    void setBounds(const Rect& bounds) { bounds_ = bounds; }
    Rect getBounds() const { return bounds_; }
    
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }
    
    void setId(const std::string& id) { id_ = id; }
    std::string getId() const { return id_; }
    
protected:
    Rect bounds_;
    bool visible_;
    bool enabled_;
    std::string id_;
};

/**
 * @brief Window container for other UI elements
 */
class Window : public UIElement {
public:
    Window(const std::string& title = "", const Rect& bounds = Rect())
        : UIElement(bounds), title_(title), draggable_(true), 
          dragging_(false), dragOffsetX_(0), dragOffsetY_(0),
          titleBarHeight_(20), backgroundColor_(0, 0, 0, 200),
          titleColor_(255, 255, 255, 255), borderColor_(255, 255, 255, 100) {}
    
    virtual ~Window() = default;
    
    void render() override;
    bool handleClick(int x, int y) override;
    bool handleHover(int x, int y) override;
    
    void addElement(const UIElementPtr& element);
    void removeElement(const UIElementPtr& element);
    void removeElementById(const std::string& id);
    UIElementPtr getElementById(const std::string& id);
    
    void setTitle(const std::string& title) { title_ = title; }
    std::string getTitle() const { return title_; }
    
    void setDraggable(bool draggable) { draggable_ = draggable; }
    bool isDraggable() const { return draggable_; }
    
    void setBackgroundColor(const Color& color) { backgroundColor_ = color; }
    Color getBackgroundColor() const { return backgroundColor_; }
    
    void setTitleColor(const Color& color) { titleColor_ = color; }
    Color getTitleColor() const { return titleColor_; }
    
    void setBorderColor(const Color& color) { borderColor_ = color; }
    Color getBorderColor() const { return borderColor_; }
    
private:
    std::string title_;
    bool draggable_;
    bool dragging_;
    int dragOffsetX_;
    int dragOffsetY_;
    int titleBarHeight_;
    Color backgroundColor_;
    Color titleColor_;
    Color borderColor_;
    std::vector<UIElementPtr> elements_;
};

/**
 * @brief Button UI element
 */
class Button : public UIElement {
public:
    Button(const std::string& text = "", const Rect& bounds = Rect())
        : UIElement(bounds), text_(text), 
          backgroundColor_(50, 50, 50, 200), 
          hoverColor_(80, 80, 80, 200),
          textColor_(255, 255, 255, 255),
          borderColor_(200, 200, 200, 100),
          isHovering_(false) {}
    
    virtual ~Button() = default;
    
    void render() override;
    bool handleClick(int x, int y) override;
    bool handleHover(int x, int y) override;
    
    void setText(const std::string& text) { text_ = text; }
    std::string getText() const { return text_; }
    
    void setOnClick(std::function<void()> onClick) { onClick_ = onClick; }
    
    void setBackgroundColor(const Color& color) { backgroundColor_ = color; }
    Color getBackgroundColor() const { return backgroundColor_; }
    
    void setHoverColor(const Color& color) { hoverColor_ = color; }
    Color getHoverColor() const { return hoverColor_; }
    
    void setTextColor(const Color& color) { textColor_ = color; }
    Color getTextColor() const { return textColor_; }
    
    void setBorderColor(const Color& color) { borderColor_ = color; }
    Color getBorderColor() const { return borderColor_; }
    
private:
    std::string text_;
    std::function<void()> onClick_;
    Color backgroundColor_;
    Color hoverColor_;
    Color textColor_;
    Color borderColor_;
    bool isHovering_;
};

/**
 * @brief Label UI element for displaying text
 */
class Label : public UIElement {
public:
    Label(const std::string& text = "", const Rect& bounds = Rect())
        : UIElement(bounds), text_(text), textColor_(255, 255, 255, 255),
          backgroundColor_(0, 0, 0, 0), fontStyle_(FontStyle::Normal) {}
    
    virtual ~Label() = default;
    
    void render() override;
    
    void setText(const std::string& text) { text_ = text; }
    std::string getText() const { return text_; }
    
    void setTextColor(const Color& color) { textColor_ = color; }
    Color getTextColor() const { return textColor_; }
    
    void setBackgroundColor(const Color& color) { backgroundColor_ = color; }
    Color getBackgroundColor() const { return backgroundColor_; }
    
    void setFontStyle(FontStyle style) { fontStyle_ = style; }
    FontStyle getFontStyle() const { return fontStyle_; }
    
private:
    std::string text_;
    Color textColor_;
    Color backgroundColor_;
    FontStyle fontStyle_;
};

/**
 * @brief Checkbox UI element
 */
class CheckBox : public UIElement {
public:
    CheckBox(const std::string& text = "", bool checked = false, const Rect& bounds = Rect())
        : UIElement(bounds), text_(text), checked_(checked),
          boxSize_(16), textColor_(255, 255, 255, 255),
          boxColor_(50, 50, 50, 200), checkColor_(0, 255, 0, 255) {}
    
    virtual ~CheckBox() = default;
    
    void render() override;
    bool handleClick(int x, int y) override;
    
    void setText(const std::string& text) { text_ = text; }
    std::string getText() const { return text_; }
    
    void setChecked(bool checked) { checked_ = checked; }
    bool isChecked() const { return checked_; }
    
    void setOnChange(std::function<void(bool)> onChange) { onChange_ = onChange; }
    
    void setTextColor(const Color& color) { textColor_ = color; }
    Color getTextColor() const { return textColor_; }
    
    void setBoxColor(const Color& color) { boxColor_ = color; }
    Color getBoxColor() const { return boxColor_; }
    
    void setCheckColor(const Color& color) { checkColor_ = color; }
    Color getCheckColor() const { return checkColor_; }
    
private:
    std::string text_;
    bool checked_;
    int boxSize_;
    std::function<void(bool)> onChange_;
    Color textColor_;
    Color boxColor_;
    Color checkColor_;
};

/**
 * @brief Menu UI element for dropdown options
 */
class Menu : public UIElement {
public:
    Menu(const std::vector<std::string>& items = {}, const Rect& bounds = Rect())
        : UIElement(bounds), items_(items), selectedIndex_(-1),
          expanded_(false), itemHeight_(20),
          backgroundColor_(50, 50, 50, 200), 
          hoverColor_(80, 80, 80, 200),
          textColor_(255, 255, 255, 255),
          borderColor_(200, 200, 200, 100),
          hoverIndex_(-1) {}
    
    virtual ~Menu() = default;
    
    void render() override;
    bool handleClick(int x, int y) override;
    bool handleHover(int x, int y) override;
    
    void setItems(const std::vector<std::string>& items) { 
        items_ = items; 
        if (selectedIndex_ >= items_.size()) {
            selectedIndex_ = -1;
        }
    }
    
    std::vector<std::string> getItems() const { return items_; }
    
    void setSelectedIndex(int index) { 
        if (index < -1 || index >= items_.size()) return;
        selectedIndex_ = index; 
        if (onChange_) onChange_(selectedIndex_);
    }
    
    int getSelectedIndex() const { return selectedIndex_; }
    
    std::string getSelectedItem() const {
        if (selectedIndex_ >= 0 && selectedIndex_ < items_.size()) {
            return items_[selectedIndex_];
        }
        return "";
    }
    
    void setOnChange(std::function<void(int)> onChange) { onChange_ = onChange; }
    
    void setBackgroundColor(const Color& color) { backgroundColor_ = color; }
    Color getBackgroundColor() const { return backgroundColor_; }
    
    void setHoverColor(const Color& color) { hoverColor_ = color; }
    Color getHoverColor() const { return hoverColor_; }
    
    void setTextColor(const Color& color) { textColor_ = color; }
    Color getTextColor() const { return textColor_; }
    
    void setBorderColor(const Color& color) { borderColor_ = color; }
    Color getBorderColor() const { return borderColor_; }
    
private:
    std::vector<std::string> items_;
    int selectedIndex_;
    bool expanded_;
    int itemHeight_;
    std::function<void(int)> onChange_;
    Color backgroundColor_;
    Color hoverColor_;
    Color textColor_;
    Color borderColor_;
    int hoverIndex_;
};

/**
 * @brief Main UI manager singleton for controlling rendering and input
 */
class UIManager {
public:
    static UIManager& getInstance();
    
    // Initialize the UI system
    bool initialize();
    
    // Cleanup resources
    void cleanup();
    
    // Window management
    WindowPtr createWindow(const std::string& title, int x, int y, int width, int height);
    void removeWindow(const WindowPtr& window);
    void removeWindowById(const std::string& id);
    WindowPtr getWindowById(const std::string& id);
    
    // Core rendering function
    void render();
    
    // Input handling
    void handleMouseMove(int x, int y);
    void handleMouseClick(int x, int y);
    void handleKeyPress(char key);
    
    // Show/hide the entire UI
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    
    // Toggle UI visibility
    void toggleVisibility() { visible_ = !visible_; }
    
private:
    UIManager() = default;
    ~UIManager() = default;
    
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;
    
    // Helper to find element at position
    UIElementPtr findElementAt(int x, int y);
    
    std::vector<WindowPtr> windows_;
    std::mutex mutex_;
    bool visible_ = true;
    bool initialized_ = false;
};

} // namespace ui
