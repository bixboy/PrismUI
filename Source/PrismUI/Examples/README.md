# PrismUI Implementation Examples

This directory contains high-performance, AAA-standard C++ examples demonstrating the core features of the PrismUI plugin.

## 1. Custom Button Extension (`UPrismExampleButton`)
Demonstrates how to extend the base button class to create unique visual behaviors.

- **Role-Based Styling**: Shows how to use the `WidgetRole` property to automatically pull design tokens (Fonts, Colors) from the active theme.
- **Visual Transitions**: Uses `StartVisualTransition` to create smooth hover/press animations without using `Tick()`.
- **AAA Performance**: Implements `NativePreConstruct` for editor-time visualization and ensures zero-tick overhead.

### Key Snippets:
```cpp
// Applying design tokens programmatically
void UPrismExampleButton::OnStyleApplied(const FPrismUIWidgetStyle& InStyleData)
{
    Super::OnStyleApplied(InStyleData);
    if (TextBlock) TextBlock->SetFont(InStyleData.Font);
}
```

## 2. Dashboard Assembly (`UPrismExampleDashboard`)
Demonstrates how to assemble complex layouts using multiple roles and the `PrismUIFactory`.

- **Fluent API Construction**: Uses `UPrismUIFactory::CreateButton` to spawn widgets dynamically.
- **State Decoupling**: Shows how to handle child events (Button clicks) in a centralized container.
- **Mixed Roles**: Demonstrates using "Primary", "Secondary", and "Danger" roles within a single interface.

### Key Snippets:
```cpp
// Spawning themed buttons dynamically
if (UPrismButtonBase* ConfirmBtn = UPrismUIFactory::CreateButton(this, ButtonClass))
{
    ConfirmBtn->SetRole(TEXT("Primary"));
    ConfirmBtn->SetText(FText::FromString(TEXT("Confirm Selection")));
}
```

## 3. Macro-Driven UI (`UPrismExampleMacroWidget`)
Demonstrates the most advanced way to build UI in C++ using the `PrismUI_Builder.h` utilities.

- **Programmatic Layout**: Shows how to construct a complete UI tree (Borders, VBox, HBox, Text) entirely in C++ without a Blueprint designer.
- **Boilerplate Reduction**: Uses macros like `PRISM_CREATE`, `PRISM_ADD_VBOX`, and `PRISM_SET_SCALE` to write concise, readable UI code.
- **Strategic Macros**: Custom header macros (e.g., `BIND_PRISM_WIDGET`) to keep member declarations clean and strictly typed.

### Key Snippets:
```cpp
// Building UI like a Pro
PRISM_CREATE_ROOT(UBorder, RootBorder);
PRISM_CREATE(UVerticalBox, MainVBox);
PRISM_ADD_OVERLAY(RootBorder, MainVBox, HAlign_Fill, VAlign_Fill, FMargin(20.0f));
```

## How to use these examples
1. **Compile**: Rebuild your project to include the new classes.
2. **Create Blueprint**: Right-click in the Content Browser and create a new Widget Blueprint.
3. **Inherit**: Search for `PrismExampleButton` or `PrismExampleDashboard` as the parent class.
4. **Bind Widgets**: In the Blueprint Designer, ensure you have a `Border` named `BackgroundBorder` and a `TextBlock` named `TextBlock` (for the button).
5. **Assign Role**: Select the widget and change the `Widget Role` in the Details panel to see the styling change instantly.

---
*For further assistance, consult the core PrismUI documentation or the @Antigravity AI.*
