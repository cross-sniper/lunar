struct AUD {
    const char *name;
    Music stream;
};

std::vector<AUD> LoadedAudio;

static int luna_load_audio(luna_State *L) {
    const char *name = lunaL_checkstring(L, 1);
    const char *path = lunaL_checkstring(L, 2);
    Music stream = LoadMusicStream(path);

    if (stream.ctxData != nullptr) {
        LoadedAudio.push_back({name, stream});
        luna_pushboolean(L, true);
    } else {
        luna_pushboolean(L, false);
    }

    return 1;
}

static int luna_play_audio(luna_State *L) {
    const char *name = lunaL_checkstring(L, 1);
    for (const AUD &audio : LoadedAudio) {
        if (strcmp(name, audio.name) == 0) {
            PlayMusicStream(audio.stream);
            return 0;
        }
    }

    // If audio is not found, you might want to handle this case
    // For now, just print a warning
    fprintf(stderr, "Audio '%s' not found\n", name);

    return 0;
}

void raylib_init_audio(luna_State *L) {
    addRaylibFunction(L, luna_load_audio, "LoadAudio");
    addRaylibFunction(L, luna_play_audio, "PlayAudio");
}
