#include "fastfetch.h"
#include "detection/gtk_qt/gtk_qt.h"
#include "common/properties.h"
#include "common/thread.h"
#include "common/settings.h"
#include "detection/displayserver/displayserver.h"

static inline bool allPropertiesSet(FFGTKResult* result)
{
    return
        result->theme.length > 0 &&
        result->icons.length > 0 &&
        result->font.length > 0;
}

static inline void applyGTKSettings(FFGTKResult* result, const char* themeName, const char* iconsName, const char* fontName, const char* cursorTheme, int cursorSize)
{
    if(result->theme.length == 0)
        ffStrbufAppendS(&result->theme, themeName);

    if(result->icons.length == 0)
        ffStrbufAppendS(&result->icons, iconsName);

    if(result->font.length == 0)
        ffStrbufAppendS(&result->font, fontName);

    if(result->cursor.length == 0)
        ffStrbufAppendS(&result->cursor, cursorTheme);

    if(result->cursorSize.length == 0 && cursorSize > 0)
        ffStrbufAppendF(&result->cursorSize, "%i", cursorSize);
}

static void detectGTKFromSettings(const FFinstance* instance, FFGTKResult* result)
{
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;

    static const char* themeName = NULL;
    static const char* iconsName = NULL;
    static const char* fontName = NULL;
    static const char* cursorTheme = NULL;
    static int cursorSize = 0;

    static bool init = false;

    ffThreadMutexLock(&mutex);

    if(init)
    {
        ffThreadMutexUnlock(&mutex);
        applyGTKSettings(result, themeName, iconsName, fontName, cursorTheme, cursorSize);
        return;
    }

    init = true;

    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, FF_DE_PRETTY_XFCE4) == 0)
    {
        themeName = ffSettingsGetXFConf(instance, "xsettings", "/Net/ThemeName", FF_VARIANT_TYPE_STRING).strValue;
        iconsName = ffSettingsGetXFConf(instance, "xsettings", "/Net/IconThemeName", FF_VARIANT_TYPE_STRING).strValue;
        fontName = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/FontName", FF_VARIANT_TYPE_STRING).strValue;
        cursorTheme = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/CursorThemeName", FF_VARIANT_TYPE_STRING).strValue;
        cursorSize = ffSettingsGetXFConf(instance, "xsettings", "/Gtk/CursorThemeSize", FF_VARIANT_TYPE_INT).intValue;
    }
    else if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, FF_DE_PRETTY_CINNAMON) == 0)
    {
        themeName = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/gtk-theme", "org.cinnamon.desktop.interface", NULL, "gtk-theme", FF_VARIANT_TYPE_STRING).strValue;
        iconsName = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/icon-theme", "org.cinnamon.desktop.interface", NULL, "icon-theme", FF_VARIANT_TYPE_STRING).strValue;
        fontName = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/font-name", "org.cinnamon.desktop.interface", NULL, "font-name", FF_VARIANT_TYPE_STRING).strValue;
        cursorTheme = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/cursor-theme", "org.cinnamon.desktop.interface", NULL, "cursor-theme", FF_VARIANT_TYPE_STRING).strValue;
        cursorSize = ffSettingsGet(instance, "/org/cinnamon/desktop/interface/cursor-size", "org.cinnamon.desktop.interface", NULL, "cursor-size", FF_VARIANT_TYPE_INT).intValue;
    }
    else if(ffStrbufIgnCaseCompS(&wmde->dePrettyName, FF_DE_PRETTY_MATE) == 0)
    {
        themeName = ffSettingsGet(instance, "/org/mate/interface/gtk-theme", "org.mate.interface", NULL, "gtk-theme", FF_VARIANT_TYPE_STRING).strValue;
        iconsName = ffSettingsGet(instance, "/org/mate/interface/icon-theme", "org.mate.interface", NULL, "icon-theme", FF_VARIANT_TYPE_STRING).strValue;
        fontName = ffSettingsGet(instance, "/org/mate/interface/font-name", "org.mate.interface", NULL, "font-name", FF_VARIANT_TYPE_STRING).strValue;
        cursorTheme = ffSettingsGet(instance, "/org/mate/peripherals-mouse/cursor-theme", "org.mate.peripherals-mouse", NULL, "cursor-theme", FF_VARIANT_TYPE_STRING).strValue;
        cursorSize = ffSettingsGet(instance, "/org/mate/peripherals-mouse/cursor-size", "org.mate.peripherals-mouse", NULL, "cursor-size", FF_VARIANT_TYPE_INT).intValue;
    }
    else if(
        ffStrbufIgnCaseCompS(&wmde->dePrettyName, FF_DE_PRETTY_GNOME) == 0 ||
        ffStrbufIgnCaseCompS(&wmde->dePrettyName, FF_DE_PRETTY_UNITY) == 0 ||
        ffStrbufIgnCaseCompS(&wmde->dePrettyName, FF_DE_PRETTY_BUDGIE) == 0
    ) {
        themeName = ffSettingsGet(instance, "/org/gnome/desktop/interface/gtk-theme", "org.gnome.desktop.interface", NULL, "gtk-theme", FF_VARIANT_TYPE_STRING).strValue;
        iconsName = ffSettingsGet(instance, "/org/gnome/desktop/interface/icon-theme", "org.gnome.desktop.interface", NULL, "icon-theme", FF_VARIANT_TYPE_STRING).strValue;
        fontName = ffSettingsGet(instance, "/org/gnome/desktop/interface/font-name", "org.gnome.desktop.interface", NULL, "font-name", FF_VARIANT_TYPE_STRING).strValue;
        cursorTheme = ffSettingsGet(instance, "/org/gnome/desktop/interface/cursor-theme", "org.gnome.desktop.interface", NULL, "cursor-theme", FF_VARIANT_TYPE_STRING).strValue;
        cursorSize = ffSettingsGet(instance, "/org/gnome/desktop/interface/cursor-size", "org.gnome.desktop.interface", NULL, "cursor-size", FF_VARIANT_TYPE_INT).intValue;
    }

    ffThreadMutexUnlock(&mutex);
    applyGTKSettings(result, themeName, iconsName, fontName, cursorTheme, cursorSize);
}

static void detectGTKFromConfigFile(const char* filename, FFGTKResult* result)
{
    ffParsePropFileValues(filename, 5, (FFpropquery[]) {
        {"gtk-theme-name =", &result->theme},
        {"gtk-icon-theme-name =", &result->icons},
        {"gtk-font-name =", &result->font},
        {"gtk-cursor-theme-name =", &result->cursor},
        {"gtk-cursor-theme-size =", &result->cursorSize}
    });
}

static void detectGTKFromConfigDir(FFstrbuf* configDir, const char* version, FFGTKResult* result)
{
    uint32_t configDirLength = configDir->length;

    // <configdir>/gtk-<version>.0/settings.ini
    ffStrbufAppendS(configDir, "gtk-");
    ffStrbufAppendS(configDir, version);
    ffStrbufAppendS(configDir, ".0/settings.ini");
    detectGTKFromConfigFile(configDir->chars, result);
    ffStrbufSubstrBefore(configDir, configDirLength);
    if(allPropertiesSet(result))
        return;

    // <configdir>/gtk-<version>.0/gtkrc
    ffStrbufAppendS(configDir, "gtk-");
    ffStrbufAppendS(configDir, version);
    ffStrbufAppendS(configDir, ".0/gtkrc");
    detectGTKFromConfigFile(configDir->chars, result);
    ffStrbufSubstrBefore(configDir, configDirLength);
    if(allPropertiesSet(result))
        return;

    // <configdir>/gtkrc-<version>.0
    ffStrbufAppendS(configDir, "gtkrc-");
    ffStrbufAppendS(configDir, version);
    ffStrbufAppendS(configDir, ".0");
    detectGTKFromConfigFile(configDir->chars, result);
    ffStrbufSubstrBefore(configDir, configDirLength);
    if(allPropertiesSet(result))
        return;

    // <configdir>/.gtkrc-<version>.0
    ffStrbufAppendS(configDir, ".gtkrc-");
    ffStrbufAppendS(configDir, version);
    ffStrbufAppendS(configDir, ".0");
    detectGTKFromConfigFile(configDir->chars, result);
    ffStrbufSubstrBefore(configDir, configDirLength);
}

static void detectGTK(const FFinstance* instance, const char* version, FFGTKResult* result)
{
    //Mate, Cinnamon, Gnome, Unity, Budgie use dconf to save theme config
    //On other DEs, this will do nothing
    detectGTKFromSettings(instance, result);
    if(allPropertiesSet(result))
        return;

    //We need to do this because we use multiple threads on configDirs
    FFstrbuf baseDir;
    ffStrbufInitA(&baseDir, 64);

    FF_LIST_FOR_EACH(FFstrbuf, configDir, instance->state.platform.configDirs)
    {
        ffStrbufSet(&baseDir, configDir);
        detectGTKFromConfigDir(&baseDir, version, result);
        if(allPropertiesSet(result))
            break;
    }

    ffStrbufDestroy(&baseDir);
}

#define FF_DETECT_GTK_IMPL(version) \
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER; \
    static FFGTKResult result; \
    static bool init = false; \
    ffThreadMutexLock(&mutex); \
    if(init){ \
        ffThreadMutexUnlock(&mutex);\
        return &result; \
    } \
    init = true; \
    ffStrbufInit(&result.theme); \
    ffStrbufInit(&result.icons); \
    ffStrbufInit(&result.font); \
    ffStrbufInit(&result.cursor); \
    ffStrbufInit(&result.cursorSize); \
    detectGTK(instance, #version, &result); \
    ffThreadMutexUnlock(&mutex); \
    return &result;

const FFGTKResult* ffDetectGTK2(const FFinstance* instance)
{
    FF_DETECT_GTK_IMPL(2)
}

const FFGTKResult* ffDetectGTK3(const FFinstance* instance)
{
    FF_DETECT_GTK_IMPL(3)
}

const FFGTKResult* ffDetectGTK4(const FFinstance* instance)
{
    FF_DETECT_GTK_IMPL(4)
}

#undef FF_CALCULATE_GTK_IMPL
