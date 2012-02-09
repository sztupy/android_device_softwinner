# inherit product from generic.mk
PRODUCT_PACKAGES := \
    AccountAndSyncSettings \
    DeskClock \
    AlarmProvider \
    Calculator \
    Calendar \
    Camera \
    CertInstaller \
    DrmProvider \
    Gallery3D \
    LatinIME \
    Launcher2 \
    Provision \
    Protips \
    QuickSearchBox \
    Settings \
    Sync \
    SystemUI \
    Updater \
    CalendarProvider \
    SyncProvider \
    Phone \
    LiveWallpapersPicker \
	SoundRecorder


PRODUCT_PROPERTY_OVERRIDES := \
    ro.config.notification_sound=OnTheHunt.ogg \
    ro.config.alarm_alert=Alarm_Classic.ogg

PRODUCT_PACKAGES += \
    bouncycastle \
    com.android.location.provider \
    com.android.location.provider.xml \
    core \
    core-junit \
    create_test_dmtrace \
    dalvikvm \
    dexdeps \
    dexdump \
    dexlist \
    dexopt \
    dmtracedump \
    dvz \
    dx \
    ext \
    framework-res \
    hprof-conv \
    icu.dat \
    jasmin \
    jasmin.jar \
    libcrypto \
    libdex \
    libdvm \
    libexpat \
    libicui18n \
    libicuuc \
    libjavacore \
    libnativehelper \
    libnfc_ndef \
    libsqlite_jni \
    libssl \
    libz \
    sqlite-jdbc \
    Browser \
    Home \
    HTMLViewer \
    ApplicationsProvider \
    ContactsProvider \
    DownloadProvider \
    DownloadProviderUi \
    MediaProvider \
    PicoTts \
    SettingsProvider \
    TelephonyProvider \
    TtsService \
    VpnServices \
    UserDictionaryProvider \
    PackageInstaller \
    DefaultContainerService \
    Bugreport

# host-only dependencies
ifeq ($(WITH_HOST_DALVIK),true)
    PRODUCT_PACKAGES += \
        bouncycastle-hostdex \
        core-hostdex \
        libjavacore-host
endif

# Pick up some sounds - stick with the short list to save space
# on smaller devices.
$(call inherit-product, frameworks/base/data/sounds/OriginalAudio.mk)

# Get the TTS language packs
$(call inherit-product-if-exists, external/svox/pico/lang/all_pico_languages.mk)

