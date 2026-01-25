#pragma once
#include <GL/glew.h>
#include <common/plugins/interfaces/render_plugin.h>

#include <map>
#include <QString>
#include <vector>

#include "textfile.h"
enum {
	SINGLE_INT = 1,
	SINGLE_FLOAT = 5,
	ARRAY_2_FLOAT = 6,
	ARRAY_3_FLOAT = 7,
	ARRAY_4_FLOAT = 8
};

struct UniformVariable {
	short type;
	short widget;
	float min;
	float max;
	float step;
	int location;
	union {
		int ival[4];
		float fval[4];
	};

	static int getVarsNumber(int i) {
		switch (i) {
		case SINGLE_INT: return 1; break;
		case SINGLE_FLOAT: return 1; break;
		case ARRAY_2_FLOAT: return 2; break;
		case ARRAY_3_FLOAT: return 3; break;
		case ARRAY_4_FLOAT: return 4; break;
		default: return 0; break;
		}
	}
};

struct TextureInfo {
	QString path;
	GLuint tId;
	short MinFilter;
	short	MagFilter;
	short	Target;
	short	WrapS;
	short	WrapT;
	short	WrapR;
};


struct ShaderInfo {
	QString vpFile;
	QString fpFile;
	std::map<QString, UniformVariable> uniformVars;
	std::map<int, QString> glStatus;
	std::vector<TextureInfo> textureInfo;
	int shaderProg;
};


enum {
	WIDGET_NONE = 0,
	WIDGET_COLOR = 1,
	WIDGET_SLIDER = 2
};

enum {
	SHADE = 0,
	ALPHA_TEST,
	ALPHA_FUNC,
	ALPHA_CLAMP,
	BLENDING,
	BLEND_FUNC_SRC,
	BLEND_FUNC_DST,
	BLEND_EQUATION,
	DEPTH_TEST,
	DEPTH_FUNC,
	CLAMP_NEAR,
	CLAMP_FAR,
	CLEAR_COLOR_R,
	CLEAR_COLOR_G,
	CLEAR_COLOR_B,
	CLEAR_COLOR_A
};

class OrthosisRender : public QObject, public RenderPlugin
{
	Q_OBJECT
	MESHLAB_PLUGIN_IID_EXPORTER(RENDER_PLUGIN_IID)
	Q_INTERFACES(RenderPlugin)

public:

	OrthosisRender();
	virtual ~OrthosisRender() {}

	QString pluginName() const;

	QList<QAction*> actions();

	bool isSupported() { return supported; }
	void refreshActions();

	void init(QAction*, MeshDocument&, MLSceneGLSharedDataContext::PerMeshRenderingDataMap&, GLArea*);
	void render(QAction*, MeshDocument&, MLSceneGLSharedDataContext::PerMeshRenderingDataMap&, GLArea*);
	void finalize(QAction*, MeshDocument*, GLArea*);

private:
	GLhandleARB v;
	GLhandleARB f;

	std::map<QString, ShaderInfo> shaders; // 存放所有shader的信息 现在Orthosis里面只有一个shader，保留这个为了以后扩展方便

	bool supported;
	QList <QAction*> actionList;

	void initActionList();
};
