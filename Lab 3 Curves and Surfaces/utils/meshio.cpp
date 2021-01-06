#include "meshio.h"

/*
* modified from: ogl-master
* common/objloader.cpp
* author: opengl-tutorial.org
*/

// anyway, we only allow one mesh and one texture here, 
// and other input may cause rendering error as a result
bool loadObj(
	bool hasDDS,
	int start0,
	int end0,
	const char* path,
	Mesh& mesh
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

	int start = start0 == -1 ? 0 : start0;
	int end = end0 == -1 ? scene->mNumMeshes : end0 + 1;
	// for now the models donnot overlap
	for (int j = start; j < end; j++)
	{
		int vertexnum = mesh.vs.size();
		const aiMesh* inmesh = scene->mMeshes[j];

		mesh.vn += inmesh->mNumVertices;
		mesh.fn += inmesh->mNumFaces;

		// Fill vertices positions
		mesh.vs.reserve(vertexnum + inmesh->mNumVertices);
		for (unsigned int i = 0; i < inmesh->mNumVertices; i++) {
			aiVector3D pos = inmesh->mVertices[i];
			mesh.vs.push_back(vec3(pos.x, pos.y, pos.z));
		}

		mesh.vts.reserve(vertexnum + inmesh->mNumVertices);
		// no texture
		if (hasDDS)
		{
			// Fill vertices texture coordinates
			for (unsigned int i = 0; i < inmesh->mNumVertices; i++) {
				aiVector3D UVW = inmesh->mTextureCoords[j][i];
				// Assume only 1 set of UV coords; AssImp supports 8 UV sets.
				mesh.vts.push_back(vec2(UVW.x, -UVW.y));
			}
		}
		else
		{
			for (unsigned int i = 0; i < inmesh->mNumVertices; i++) {
				mesh.vts.push_back(vec2(0, 0));
			}
		}

		// Fill vertices normals
		mesh.vns.reserve(vertexnum + inmesh->mNumVertices);
		for (unsigned int i = 0; i < inmesh->mNumVertices; i++) {
			aiVector3D n = inmesh->mNormals[i];
			mesh.vns.push_back(glm::vec3(n.x, n.y, n.z));
		}


		// Fill face indices
		mesh.fs.reserve(mesh.fs.size() + 3 * inmesh->mNumFaces);
		mesh.maxvs.reserve(mesh.maxvs.size() + inmesh->mNumFaces);
		mesh.minvs.reserve(mesh.minvs.size() + inmesh->mNumFaces);
		mesh.ns.reserve(mesh.ns.size() + inmesh->mNumFaces);
		for (unsigned int i = 0; i < inmesh->mNumFaces; i++) {
			// Assume the model has only triangles.
			mesh.fs.push_back(inmesh->mFaces[i].mIndices[0] + vertexnum);
			mesh.fs.push_back(inmesh->mFaces[i].mIndices[1] + vertexnum);
			mesh.fs.push_back(inmesh->mFaces[i].mIndices[2] + vertexnum);
			vec3 v1 = mesh.vs[inmesh->mFaces[i].mIndices[0] + vertexnum];
			vec3 v2 = mesh.vs[inmesh->mFaces[i].mIndices[1] + vertexnum];
			vec3 v3 = mesh.vs[inmesh->mFaces[i].mIndices[2] + vertexnum];
			mesh.maxvs.push_back(glm::max(glm::max(v1, v2), v3));
			mesh.minvs.push_back(glm::min(glm::min(v1, v2), v3));
			// no interpolation for edges 
			vec3 n1 = mesh.vns[inmesh->mFaces[i].mIndices[0] + vertexnum];
			vec3 n2 = mesh.vns[inmesh->mFaces[i].mIndices[1] + vertexnum];
			vec3 n3 = mesh.vns[inmesh->mFaces[i].mIndices[2] + vertexnum];
			mesh.ns.push_back(glm::normalize(n1 + n2 + n3));
		}

	}

	return true;
}

// 6-837 MIT computer graphics basic code
void outputObjFile(std::string name, const vector<vec3>& vertex, const vector<vec3>& normal)
{
	std::ofstream out(name.c_str());

	if (!out)
	{
		std::cout<<name.c_str()<<"cannot be opened";
	}
	else
	{
		std::cout << "writing to "<<name.c_str() ;
		for (unsigned i = 0; i < vertex.size(); i++)
			out << "v  "
			<< vertex[i][0] << " "
			<< vertex[i][1] << " "
			<< vertex[i][2] << std::endl;

		for (unsigned i = 0; i < normal.size(); i++)
			out << "vn "
			<< normal[i][0] << " "
			<< normal[i][1] << " "
			<< normal[i][2] << std::endl;

		out << "vt  0 0 0" << std::endl;

		int t = 0;
		for (unsigned i = 0; i < vertex.size() / 3; i++)
		{
			out << "f  ";
			for (unsigned j = 0; j < 3; j++)
			{
				t += 1;
				out << t << "/" << 1 << "/" << t << " ";
			}
			out << std::endl;
		}
		std::cout <<"wrote " << name << std::endl;
	}
	
}

/*
* modified from: ogl-master
* common/shader.cpp
* author: opengl-tutorial.org
*/
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
