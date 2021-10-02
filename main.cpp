#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QVariant>
#include <utility>

#include "dimg/core/dplugins/dimg/heif/dimgheifplugin.h"
#include "dimg/core/dplugins/dimg/imagemagick/dimgimagemagickplugin.h"
#include "dimg/core/dplugins/dimg/jpeg/dimgjpegplugin.h"
#include "dimg/core/dplugins/dimg/jpeg2000/dimgjpeg2000plugin.h"
#include "dimg/core/dplugins/dimg/png/dimgpngplugin.h"
#include "dimg/core/dplugins/dimg/tiff/dimgtiffplugin.h"
#include "dimg/core/dplugins/dimg/qimage/dimgqimageplugin.h"
#include "dimg/core/dplugins/dimg/raw/dimgrawplugin.h"
#include "dimg/core/dplugins/dimg/pgf/dimgpgfplugin.h"
#include "dimg/shims/include/dpluginloader.h"

#include "item.h"

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    Digikam::DPluginLoader::addPlugin(new DigikamHEIFDImgPlugin::DImgHEIFPlugin{});
    Digikam::DPluginLoader::addPlugin(new DigikamImageMagickDImgPlugin::DImgImageMagickPlugin{});
    Digikam::DPluginLoader::addPlugin(new DigikamJPEG2000DImgPlugin::DImgJPEG2000Plugin{});
    Digikam::DPluginLoader::addPlugin(new DigikamJPEGDImgPlugin::DImgJPEGPlugin{});
    Digikam::DPluginLoader::addPlugin(new DigikamPNGDImgPlugin::DImgPNGPlugin{});
    Digikam::DPluginLoader::addPlugin(new DigikamQImageDImgPlugin::DImgQImagePlugin{});
    Digikam::DPluginLoader::addPlugin(new DigikamTIFFDImgPlugin::DImgTIFFPlugin{});
    Digikam::DPluginLoader::addPlugin(new DigikamRAWDImgPlugin::DImgRAWPlugin{});
    Digikam::DPluginLoader::addPlugin(new DigikamPGFDImgPlugin::DImgPGFPlugin{});

    qmlRegisterType<DImgViewer>("DImgViewer", 1, 0, "DImgViewer");

    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:/");
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated,
      &app, [url](QObject* obj, const QUrl& objUrl) {
          if(!obj && url == objUrl)
              QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
