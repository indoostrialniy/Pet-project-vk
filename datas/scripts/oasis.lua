--~ if (Slovo == "type") 	{								rookie.initType 	= stof(getValue(StrVar, 1)); }
--~ if (Slovo == "sysname") { 								rookie.initSysname 	= getValue(StrVar, 1); }
--~ if (Slovo == "position") { for (int i = 0; i < 3; i++) 	rookie.initPos[i] 	= stof(getValue(StrVar, i + 1)); }
--~ if (Slovo == "direction")      //выдираем кватернион поворота
--~ {
														--~ rookie.initRot.w 	= stof(getValue(StrVar, 1));
														--~ rookie.initRot.x 	= stof(getValue(StrVar, 2));
														--~ rookie.initRot.y 	= stof(getValue(StrVar, 3));
														--~ rookie.initRot.z 	= stof(getValue(StrVar, 4));
--~ }
--~ if (Slovo == "scale") { for (int i = 0; i < 3; i++) 	rookie.initScale[i] 	= stof(getValue(StrVar, i + 1)); }
--~ if (Slovo == "model") { 								rookie.initMeshName = "datas//meshes/"+getValue(StrVar, 1)+".model"; }
--~ if (Slovo == "material") { 								rookie.initMeshMaterialsVector.push_back(getValue(StrVar, 1)); }

--~ FhEntityInitializationInfo rookie;		

--~ void addComponentToMicroWorld( const std::string& actorName, FhEntityInitializationInfo& newEntity );


--~ struct FhEntityInitializationInfo
--~ {
--~ public:
	--~ std::string initSysname; 											// системное имя, уникальное в пределах актора
	--~ std::string initMeshName; 											// model filename
	--~ glm::vec3 	initPos			= glm::vec3(0.0f, 0.0f, 0.0f); 			// локальные координаты сущности внутри актора
	--~ glm::quat 	initRot			= glm::quat(1.0f, 0.0f, 0.0f, 0.0f);	// локальное вращение сущности внутри актора
	--~ glm::vec3 	initScale		= glm::vec3(1.0f, 1.0f, 1.0f);
	--~ uint32_t 	initType 		= FH_ENTITY_TYPE_RENDER_OPAQUE;
	--~ std::vector <std::string> 	initMeshMaterialsVector;				// вектор текстовых имен назначенных материалов
--~ };




--~ local s = foo(2, 3)
--~ print(s)  -- 5

--~ local l = renderer_setViewport(800, 600)


--~ local name = "Front"
--~ local model = "Front"
--~ local material = "Material_3"

local v1 = vec3(1, 2, 3)
local v2 = vec3(4, 5, 6)
local v3 = v1 + v2          -- работает благодаря meta_function::addition

print v3.x
