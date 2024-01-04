Camera3D Cam;

struct ModelType {
	const char *name;
	Model model;
};
std::vector<ModelType> LoadedModels;
static int luna_load_model_3d(luna_State *L) {
	Model model;
	const char *path = lunaL_checkstring(L, 1);
	const char *name = lunaL_checkstring(L, 2);
	//load a .obj using raylib's function
	model = LoadModel(path);
	ModelType new_model = {name, model};

	LoadedModels.push_back(new_model);
	return 0;
}


static int luna_draw_model_3d(luna_State *L) {
	const char *name = lunaL_checkstring(L, 1);
	for (int i = 0; i < LoadedModels.size(); i++) {
		if (strcmp(name, LoadedModels[i].name) == 0) {
			DrawModel(LoadedModels[i].model, Vector3{ 0, 0, 0 }, 1, WHITE);
		}
	}
	return 0;
}

static int luna_begin_3d(luna_State *L) {

	BeginMode3D(Cam);
	return 0;
}
static int luna_end_3d(luna_State *L) {
	EndMode3D();
	return 0;
}


static int raylib_init_3d(luna_State *L) {
	Cam.position = Vector3{ 0, 0, 0 };
	Cam.target = Vector3{ 0, 0, 0 };
	Cam.up = Vector3{ 0, 1, 0 };
	//Cam.projection = CAMERA_PERSPECTIVE;

	addRaylibFunction(L, luna_load_model_3d, "LoadModel");
	addRaylibFunction(L, luna_draw_model_3d, "DrawModel");
	addRaylibFunction(L, luna_begin_3d, "BeginMode3D");
	addRaylibFunction(L, luna_end_3d, "EndMode3D");
	return 1;
}