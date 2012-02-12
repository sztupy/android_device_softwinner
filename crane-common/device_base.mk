# host-only dependencies
ifeq ($(WITH_HOST_DALVIK),true)
    PRODUCT_PACKAGES += \
        bouncycastle-hostdex \
        core-hostdex \
        libjavacore-host
endif

# Get the TTS language packs
$(call inherit-product-if-exists, build/target/product/full_base.mk)

