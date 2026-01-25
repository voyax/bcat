#include <math.h>
#include <stdlib.h>

#include <common/GLExtensionsManager.h>
#include <common/globals.h>
#include <common/mlapplication.h>
#include "orthosisRender.h"

#include <QGLWidget>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include "../../meshlab/glarea.h"

OrthosisRender::OrthosisRender() {
	initActionList();
}

QString OrthosisRender::pluginName() const
{
	return QString("orthosis render");
}

QList<QAction*> OrthosisRender::actions()
{
	return actionList;
}

void OrthosisRender::refreshActions()
{
}

void OrthosisRender::init(QAction* a, MeshDocument&/*md*/, MLSceneGLSharedDataContext::PerMeshRenderingDataMap& /*mp*/, GLArea* gla)
{
	gla->makeCurrent();
	if (GLExtensionsManager::initializeGLextensions_notThrowing()) {
		if (GLEW_ARB_vertex_program && GLEW_ARB_fragment_program) {
			supported = true;
			if (shaders.find(a->text()) != shaders.end()) {

				v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
				f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

				char* fs = textFileRead((shaders[a->text()].fpFile).toLocal8Bit().data());
				char* vs = textFileRead((shaders[a->text()].vpFile).toLocal8Bit().data());

				const char* vv = vs;
				const char* ff = fs;
				glShaderSourceARB(v, 1, &vv, NULL);
				glShaderSourceARB(f, 1, &ff, NULL);

				free(fs);
				free(vs);

				glCompileShaderARB(v);
				glCompileShaderARB(f);

				GLint statusV;
				GLint statusF;

				glGetObjectParameterivARB(v, GL_OBJECT_COMPILE_STATUS_ARB, &statusV);
				glGetObjectParameterivARB(f, GL_OBJECT_COMPILE_STATUS_ARB, &statusF);

				if (statusF && statusV) { //successful compile
					shaders[a->text()].shaderProg = glCreateProgramObjectARB();

					glAttachObjectARB(shaders[a->text()].shaderProg, v);
					glAttachObjectARB(shaders[a->text()].shaderProg, f);
					glLinkProgramARB(shaders[a->text()].shaderProg);

					GLint linkStatus;
					glGetObjectParameterivARB(shaders[a->text()].shaderProg, GL_OBJECT_LINK_STATUS_ARB, &linkStatus);

					if (linkStatus) {
						std::map<QString, UniformVariable>::iterator i = shaders[a->text()].uniformVars.begin();
						while (i != shaders[a->text()].uniformVars.end()) {
							(shaders[a->text()].uniformVars[i->first]).location = glGetUniformLocationARB(shaders[a->text()].shaderProg, (i->first).toLocal8Bit().data());
							++i;
						}

					}
					else
					{
						QFile file("orthosis_shaders.log");
						if (file.open(QFile::Append))
						{
							char proglog[2048];
							GLsizei length;
							QTextStream out(&file);

							glGetProgramiv(v, GL_LINK_STATUS, &statusV);
							glGetProgramInfoLog(v, 2048, &length, proglog);
							out << "VERTEX SHADER LINK INFO:" << endl;
							out << proglog << endl << endl;

							glGetProgramiv(f, GL_LINK_STATUS, &statusF);
							glGetProgramInfoLog(f, 2048, &length, proglog);
							out << "FRAGMENT SHADER LINK INFO:" << endl << endl;
							out << proglog << endl << endl;

							file.close();
						}

						QMessageBox::critical(0, "Meshlab",
							QString("An error occurred during shader's linking.\n") +
							"See shaders.log for further details about this error.\n");
					}

					//Textures
					std::vector<TextureInfo>::iterator tIter = shaders[a->text()].textureInfo.begin();
					while (tIter != shaders[a->text()].textureInfo.end())
					{
						glEnable(tIter->Target);
						QImage img, imgScaled, imgGL;
						bool opened = img.load(tIter->path);
						if (!opened)
						{
							supported = false;
							return;
						}
						// image has to be scaled to a 2^n size. We choose the first 2^N <= picture size.
						int bestW = pow(2.0, floor(::log(double(img.width())) / ::log(2.0)));
						int bestH = pow(2.0, floor(::log(double(img.height())) / ::log(2.0)));
						imgScaled = img.scaled(bestW, bestH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
						imgGL = QGLWidget::convertToGLFormat(imgScaled);

						glGenTextures(1, &(tIter->tId));
						glBindTexture(tIter->Target, tIter->tId);
						glTexImage2D(tIter->Target, 0, 3, imgGL.width(), imgGL.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imgGL.bits());
						glTexParameteri(tIter->Target, GL_TEXTURE_MIN_FILTER, tIter->MinFilter);
						glTexParameteri(tIter->Target, GL_TEXTURE_MAG_FILTER, tIter->MagFilter);
						glTexParameteri(tIter->Target, GL_TEXTURE_WRAP_S, tIter->WrapS);
						glTexParameteri(tIter->Target, GL_TEXTURE_WRAP_T, tIter->WrapT);
						glTexParameteri(tIter->Target, GL_TEXTURE_WRAP_R, tIter->WrapR);

						++tIter;
					}
				}
				else
				{
					QFile file("orthosis_shaders.log");
					if (file.open(QFile::WriteOnly))
					{
						char shlog[2048];
						GLsizei length;
						QTextStream out(&file);

						glGetShaderiv(v, GL_COMPILE_STATUS, &statusV);
						glGetShaderInfoLog(v, 2048, &length, shlog);
						out << "VERTEX SHADER COMPILE INFO:" << endl << endl;
						out << shlog << endl << endl;

						glGetShaderiv(f, GL_COMPILE_STATUS, &statusF);
						glGetShaderInfoLog(f, 2048, &length, shlog);
						out << "FRAGMENT SHADER COMPILE INFO:" << endl << endl;
						out << shlog << endl << endl;

						file.close();
					}

					QMessageBox::critical(0, "Meshlab",
						QString("An error occurred during shader's compiling.\n"
							"See shaders.log for further details about this error."));
				}
			}
		}
	}

	// * clear the errors, if any
	glGetError();
}

void OrthosisRender::render(QAction* a, MeshDocument& md, MLSceneGLSharedDataContext::PerMeshRenderingDataMap& /*mp*/, GLArea* gla)
{
	//  MeshModel &mm
	if (shaders.find(a->text()) != shaders.end()) {
		ShaderInfo si = shaders[a->text()];

		glUseProgramObjectARB(si.shaderProg);

		glUniform1fARB(si.uniformVars[QString("edgefalloff")].location,	si.uniformVars[QString("edgefalloff")].fval[0]);
		glUniform1fARB(si.uniformVars[QString("intensity")].location,	si.uniformVars[QString("intensity")].fval[0]);
		glUniform1fARB(si.uniformVars[QString("ambient")].location,		si.uniformVars[QString("ambient")].fval[0]);

		std::map<int, QString>::iterator j = si.glStatus.begin();
		while (j != si.glStatus.end()) {
			switch (j->first) {
			case SHADE: glShadeModel(j->second.toInt()); break;
			case ALPHA_TEST: if (j->second == "True") glEnable(GL_ALPHA_TEST); else glDisable(GL_ALPHA_TEST); break;
			case ALPHA_FUNC: glAlphaFunc(j->second.toInt(), (si.glStatus[ALPHA_CLAMP]).toFloat()); break;
			case BLENDING: if (j->second == "True") glEnable(GL_BLEND); else glDisable(GL_BLEND); break;
			case BLEND_FUNC_SRC: glBlendFunc(j->second.toInt(), (si.glStatus[BLEND_FUNC_DST]).toInt()); break;
			case BLEND_EQUATION: glBlendEquation(j->second.toInt()); break;
			case DEPTH_TEST: if (j->second == "True") glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST); break;
			case DEPTH_FUNC: glDepthFunc(j->second.toInt()); break;
			case CLEAR_COLOR_R: glClearColor(j->second.toFloat(),
				(si.glStatus[CLEAR_COLOR_G]).toFloat(),
				(si.glStatus[CLEAR_COLOR_B]).toFloat(),
				(si.glStatus[CLEAR_COLOR_A]).toFloat()); break;
			}
			++j;
		}

		int n = GL_TEXTURE0_ARB;
		std::vector<TextureInfo>::iterator tIter = shaders[a->text()].textureInfo.begin();
		while (tIter != shaders[a->text()].textureInfo.end()) {
			glActiveTexture(n);
			glEnable(tIter->Target);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			glBindTexture(tIter->Target, tIter->tId);
			++tIter;
			++n;
		}

		if ((gla != NULL) && (gla->mvc() != NULL)) {
			MLSceneGLSharedDataContext* shared = gla->mvc()->sharedDataContext();
			glUniform1iARB(si.uniformVars[QString("istransparent")].location, 0);
			glDepthMask(GL_TRUE);
			for (const MeshModel& mp : md.meshIterator()) {
				if (!mp.isTransparent) { //先绘制不透明物体
					if (gla->meshVisibilityMap[mp.id()]) {
						shared->draw(mp.id(), gla->context());
					}
				}
			}

			glUniform1iARB(si.uniformVars[QString("istransparent")].location, 1);
			glDepthMask(GL_FALSE);
			for (const MeshModel& mp : md.meshIterator()) {
				if (mp.isTransparent) { //再绘制透明物体
					if (gla->meshVisibilityMap[mp.id()]) {
						shared->draw(mp.id(), gla->context());
					}
				}
			}
			glDepthMask(GL_TRUE);
		}
	}
	// * clear the errors, if any
	glGetError();

	glUseProgramObjectARB(0);
}

void OrthosisRender::finalize(QAction*, MeshDocument*, GLArea*)
{
}

void OrthosisRender::initActionList()
{
	//获取shader文件的基础路径，自定义的shader文件质保函.vert和.frag
	QDir shadersDir = QDir(meshlab::defaultShadersPath());
	ShaderInfo si;

	// path for vertex shader and fragment shader
	si.vpFile = shadersDir.absoluteFilePath(QString("orthosis.vert"));
	si.fpFile = shadersDir.absoluteFilePath(QString("orthosis.frag"));

	//uniform 变量赋值
	UniformVariable istransparent;
	istransparent.type	= 1;
	istransparent.ival[0]	= 1;
	si.uniformVars[QString("istransparent")] = istransparent;

	// shade state 赋值
	si.glStatus[SHADE]			= "7425";
	si.glStatus[ALPHA_TEST]		= "False";
	si.glStatus[ALPHA_FUNC]		= "516";
	si.glStatus[ALPHA_CLAMP]	= "0.5";
	si.glStatus[BLENDING]		= "True";
	si.glStatus[BLEND_FUNC_SRC] = "770";
	si.glStatus[BLEND_FUNC_DST] = "771";
	si.glStatus[BLEND_EQUATION] = "32774";
	si.glStatus[DEPTH_TEST]		= "True";
	si.glStatus[DEPTH_FUNC]		= "515";
	si.glStatus[CLAMP_NEAR]		= "0";
	si.glStatus[CLAMP_FAR]		= "1";
	si.glStatus[CLEAR_COLOR_R] = "0.5019608";
	si.glStatus[CLEAR_COLOR_G] = "0.5019608";
	si.glStatus[CLEAR_COLOR_B] = "0.5019608";
	si.glStatus[CLEAR_COLOR_A] = "1";

	shaders["orthosis"] = si;

	//initial actions
	QAction* qa = new QAction("orthosis", this);
	qa->setCheckable(false);
	actionList << qa;
}

MESHLAB_PLUGIN_NAME_EXPORTER(OrthosisRender)
