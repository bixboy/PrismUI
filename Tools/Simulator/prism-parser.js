/**
 * PrismUI C++ Pattern Parser
 * Analyzes Unreal C++ source code to extract UI hierarchies.
 */

const fs = require('fs');
const path = require('path');

class PrismParser {
    constructor() {
        this.widgets = new Map();
        this.hierarchy = [];
        this.rootWidgets = new Set();
    }

    /**
     * Parses a .cpp and .h file pair.
     */
    parseFile(cppPath) {
        this.widgets.clear();
        this.hierarchy = [];
        this.rootWidgets.clear();

        const cppContent = fs.readFileSync(cppPath, 'utf8');
        let hContent = '';
        
        // Unreal Public/Private resolution
        let hPath = cppPath.replace('.cpp', '.h');
        if (!fs.existsSync(hPath)) {
            hPath = cppPath.replace(/\\Private\\|\\\\Private\\\\|\/Private\//, '/Public/').replace('.cpp', '.h');
        }
        
        if (fs.existsSync(hPath)) {
            console.log(`[Prism] Found header: ${hPath}`);
            hContent = fs.readFileSync(hPath, 'utf8');
        } else {
            console.log(`[Prism] Header not found for ${cppPath}`);
        }

        // 1. Extract types from Header
        this.extractTypes(hContent);
        
        // 2. Extract types from CPP (locals)
        this.extractTypes(cppContent);

        // 3. Extract Allocations & Properties
        this.extractLogic(cppContent);

        return this.buildTree();
    }

    extractTypes(content) {
        // Match: UButton* MyButton; or TObjectPtr<UButton> MyButton; or UButton *MyButton;
        const typeRegex = /(?:TObjectPtr<|class\s+)?([A-Z][a-zA-Z0-9_]+)\s*\*?\s*>?\s+([a-zA-Z0-9_]+)\s*(?:=|;|\))/g;
        let match;
        while ((match = typeRegex.exec(content)) !== null) {
            const [_, type, name] = match;
            if (!this.widgets.has(name) && name !== 'TEXT') {
                this.widgets.set(name, { id: name, type: type, props: {}, children: [] });
            }
        }
    }

    extractLogic(content) {
        // Match allocations: Name = NewObject<Type>(...) or Name = Factory::Create(...)
        // Handle both: Name = ... and Type* Name = ...
        // Generalized to support any *UIFactory (Prism, Jupiter, etc.)
        // Support for PRISM_CREATE macro
        const allocRegex = /(?:PRISM_CREATE\s*\(\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*\)|([a-zA-Z0-9_]+)\s*=\s*(?:NewObject<([A-Z][a-zA-Z0-9_]+)>|([a-zA-Z0-9_]+)UIFactory::Create([a-zA-Z0-9_]+)\((.*?)\)|CreateWidget<([A-Z][a-zA-Z0-9_]+)>))/g;
        let match;
        while ((match = allocRegex.exec(content)) !== null) {
            const [_, macroType, macroName, nameRaw, nativeType, factoryModule, factoryType, factoryArgs, widgetType] = match;
            const name = macroName || nameRaw;
            let type = macroType || nativeType || widgetType;
            
            if (!type && factoryType) {
                type = factoryType.includes('Button') ? 'UButton' : ('U' + factoryType);
            }
            
            if (!this.widgets.has(name)) {
                this.widgets.set(name, { id: name, type: type || 'UWidget', props: {}, children: [] });
            } else if (type) {
                this.widgets.get(name).type = type;
            }

            // Extract labels/text from Factory arguments (heuristic)
            if (factoryArgs) {
                const args = factoryArgs.split(',').map(a => a.trim());
                if (factoryType === 'ParameterBox' && args.length >= 2) {
                    this.widgets.get(name).props.label = this.cleanValue(args[1]);
                    // If Arg 3 is a variable name, link it as child
                    const potentialChild = args[2];
                    if (potentialChild && /^[a-zA-Z0-9_]+$/.test(potentialChild)) {
                        this.widgets.get(name).children.push(potentialChild);
                        this.rootWidgets.delete(potentialChild);
                    }
                } else if (factoryType.includes('Button') && args.length >= 2) {
                    this.widgets.get(name).props.text = this.cleanValue(args[1]);
                }
            }
        }

        // Match properties: Name->SetText(...)
        const propRegex = /([a-zA-Z0-9_]+)->Set([a-zA-Z0-9_]+)\((.*?)\)/g;
        while ((match = propRegex.exec(content)) !== null) {
            const [_, name, prop, value] = match;
            if (this.widgets.has(name)) {
                const cleanValue = this.cleanValue(value);
                this.widgets.get(name).props[prop.toLowerCase()] = cleanValue;
            }
        }

        // Match hierarchy: Parent->AddChildToVerticalBox(Child)
        // Support for PRISM_ADD_VBOX, PRISM_ADD_HBOX, PRISM_ADD_OVERLAY macros
        const hierarchyRegex = /(?:([a-zA-Z0-9_]+)->AddChildTo(?:VerticalBox|HorizontalBox|Canvas|Panel|Stack|Overlay)\((.*?)\)|PRISM_ADD_(?:VBOX|HBOX|OVERLAY|SIZEBOX)\s*\(\s*([a-zA-Z0-9_]+)\s*,\s*([a-zA-Z0-9_]+)\s*,?)/g;
        while ((match = hierarchyRegex.exec(content)) !== null) {
            const [_, parentRaw, childRaw, macroParent, macroChild] = match;
            const parent = parentRaw || macroParent;
            const child = (childRaw || macroChild).split(',')[0].trim();
            
            if (!this.widgets.has(parent)) {
                this.widgets.set(parent, { id: parent, type: 'UWidget', props: {}, children: [] });
            }

            if (this.widgets.has(child)) {
                if (!this.widgets.get(parent).children.includes(child)) {
                    this.widgets.get(parent).children.push(child);
                }
                this.rootWidgets.delete(child);
            }
            
            if (!this.isChild(parent)) {
                this.rootWidgets.add(parent);
            }
        }
    }

    cleanValue(value) {
        // Extract string from FText::FromString(TEXT("...")) or TEXT("...")
        const textMatch = /TEXT\("([^"]+)"\)/.exec(value) || /"([^"]+)"/.exec(value);
        if (textMatch) return textMatch[1];
        
        // Handle Enums
        const enumMatch = /::([a-zA-Z0-9_]+)/.exec(value);
        if (enumMatch) return enumMatch[1];

        return value.trim();
    }

    isChild(name) {
        for (const w of this.widgets.values()) {
            if (w.children.includes(name)) return true;
        }
        return false;
    }

    buildTree() {
        const roots = [];
        for (const name of this.rootWidgets) {
            roots.push(this.resolveNode(name));
        }
        return roots;
    }

    resolveNode(name) {
        const widget = this.widgets.get(name);
        if (!widget) return null;

        return {
            ...widget,
            children: widget.children.map(c => this.resolveNode(c)).filter(n => n !== null)
        };
    }
}

module.exports = PrismParser;
