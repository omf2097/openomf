#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)

#ifdef V_LABEL
#define LABEL_STR V_LABEL
#else
#define LABEL_STR ""
#endif

static const char *_version_string =
    EXPAND_AND_STRINGIFY(V_MAJOR) "." EXPAND_AND_STRINGIFY(V_MINOR) "." EXPAND_AND_STRINGIFY(V_PATCH) LABEL_STR;

const char *get_version_string() {
    return _version_string;
}

int get_version_major() {
    return V_MAJOR;
}

int get_version_minor() {
    return V_MINOR;
}

int get_version_patch() {
    return V_MINOR;
}

char *get_version_label() {
    return LABEL_STR;
}
