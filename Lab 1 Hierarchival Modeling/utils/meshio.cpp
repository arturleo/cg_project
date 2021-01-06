#include "meshio.h"

Mesh merge(Mesh a, Mesh b)
{
	Mesh c = a;
	c.fn += b.fn;
	c.vn += b.vn;
	c.vs.reserve(c.vn);
	c.vs.insert(c.vs.end(), b.vs.begin(), b.vs.end());
	c.vts.reserve(c.vn);
	c.vts.insert(c.vts.end(), b.vts.begin(), b.vts.end());
	c.vns.reserve(c.vn);
	c.vns.insert(c.vns.end(), b.vns.begin(), b.vns.end());
	c.fs.reserve(c.fn * 3);
	for (int i = 0; i < b.fn; i++)
	{
		c.fs.push_back(b.fs[i * 3] + a.vn);
		c.fs.push_back(b.fs[i * 3 + 1] + a.vn);
		c.fs.push_back(b.fs[i * 3 + 2] + a.vn);
	}
	return c;
}

/*
* modified from: ogl-master
* common/objloader.cpp
* author: opengl-tutorial.org
*/
// load multiple meshes
bool loadObj(
	bool hasDDS,
	const char* path,
	vector<Mesh>& meshes,
	Mesh& mesh0
) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		path,
		0		/*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/
	);
	if (!scene) {
		fprintf(stderr, importer.GetErrorString());
		getchar();
		return false;
	}

	// for now the models donnot overlap
	for (int j = 0; j < scene->mNumMeshes; j++)
	{
		Mesh mesh;
		int vertexnum = mesh0.vs.size();
		const aiMesh* inmesh = scene->mMeshes[j];

		mesh.vn += inmesh->mNumVertices;
		mesh.fn += inmesh->mNumFaces;
		mesh0.vn += inmesh->mNumVertices;
		mesh0.fn += inmesh->mNumFaces;

		// Fill vertices positions
		mesh0.vs.reserve(vertexnum + inmesh->mNumVertices);
		mesh.vs.reserve(inmesh->mNumVertices);
		for (unsigned int i = 0; i < inmesh->mNumVertices; i++) {
			aiVector3D pos = inmesh->mVertices[i];
			mesh.vs.push_back(vec3(pos.x, pos.y, pos.z));
			mesh0.vs.push_back(vec3(pos.x, pos.y, pos.z));
		}

		mesh.vts.reserve(inmesh->mNumVertices);
		mesh0.vts.reserve(vertexnum + inmesh->mNumVertices);
		// no texture
		if (hasDDS)
		{
			// Fill vertices texture coordinates
			for (unsigned int i = 0; i < inmesh->mNumVertices; i++) {
				aiVector3D UVW = inmesh->mTextureCoords[j][i];
				// Assume only 1 set of UV coords; AssImp supports 8 UV sets.
				mesh.vts.push_back(vec2(UVW.x, -UVW.y));
				mesh0.vts.push_back(vec2(UVW.x, -UVW.y));
			}
		}
		else
		{
			for (unsigned int i = 0; i < inmesh->mNumVertices; i++) {
				mesh.vts.push_back(vec2(0, 0));
				mesh0.vts.push_back(vec2(0, 0));
			}
		}

		// Fill vertices normals
		mesh.vns.reserve(inmesh->mNumVertices);
		mesh0.vns.reserve(vertexnum + inmesh->mNumVertices);
		for (unsigned int i = 0; i < inmesh->mNumVertices; i++) {
			aiVector3D n = inmesh->mNormals[i];
			mesh.vns.push_back(glm::vec3(n.x, n.y, n.z));
			mesh0.vns.push_back(glm::vec3(n.x, n.y, n.z));
		}

		// Fill face indices
		mesh.fs.reserve( 3 * inmesh->mNumFaces);
		mesh0.fs.reserve(mesh0.fs.size() + 3 * inmesh->mNumFaces);
		for (unsigned int i = 0; i < inmesh->mNumFaces; i++) {
			// Assume the model has only triangles.
			mesh.fs.push_back(inmesh->mFaces[i].mIndices[0]);
			mesh.fs.push_back(inmesh->mFaces[i].mIndices[1]);
			mesh.fs.push_back(inmesh->mFaces[i].mIndices[2]);
			mesh0.fs.push_back(inmesh->mFaces[i].mIndices[0] + vertexnum);
			mesh0.fs.push_back(inmesh->mFaces[i].mIndices[1] + vertexnum);
			mesh0.fs.push_back(inmesh->mFaces[i].mIndices[2] + vertexnum);
		}
		meshes.push_back(mesh);
	}

	return true;
};


void LoadShader(GLuint ShaderID, const char* file_path)
{
	// Read the Vertex Shader code from the file
	std::string ShaderCode;
	std::ifstream ShaderStream(file_path, std::ios::in);
	if (ShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << ShaderStream.rdbuf();
		ShaderCode = sstr.str();
		ShaderStream.close();
	}
	else {
		printf("Impossible to open %s. \n", file_path);
		getchar();
		exit(-1);
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	printf("Compiling shader : %s\n", file_path);
	char const* xSourcePointer = ShaderCode.c_str();
	glShaderSource(ShaderID, 1, &xSourcePointer, NULL);
	glCompileShader(ShaderID);

	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, &ShaderErrorMessage[0]);
		printf("%s\n", &ShaderErrorMessage[0]);
	}
}



GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	LoadShader(VertexShaderID, vertex_file_path);
	LoadShader(FragmentShaderID, fragment_file_path);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
