This is the current MacOS dist/ structure:

```
macOS:~ mjesbar$ tree ~/gladys/dist/ -L 4
/Users/mjesbar/gladys/dist/
└── gladys.app
    └── Contents
        ├── Frameworks
        │   ├── QtCore.framework
        │   ├── QtDBus.framework
        │   ├── QtGui.framework
        │   ├── QtHttpServer.framework
        │   ├── QtNetwork.framework
        │   ├── QtOpenGL.framework
        │   ├── QtQml.framework
        │   ├── QtQmlLocalStorage.framework
        │   ├── QtQmlMeta.framework
        │   ├── QtQmlModels.framework
        │   ├── QtQmlWorkerScript.framework
        │   ├── QtQmlXmlListModel.framework
        │   ├── QtQuick.framework
        │   ├── QtQuickControls2.framework
        │   ├── QtQuickControls2Basic.framework
        │   ├── QtQuickControls2BasicStyleImpl.framework
        │   ├── QtQuickControls2FluentWinUI3StyleImpl.framework
        │   ├── QtQuickControls2Fusion.framework
        │   ├── QtQuickControls2FusionStyleImpl.framework
        │   ├── QtQuickControls2IOSStyleImpl.framework
        │   ├── QtQuickControls2Imagine.framework
        │   ├── QtQuickControls2ImagineStyleImpl.framework
        │   ├── QtQuickControls2Impl.framework
        │   ├── QtQuickControls2MacOSStyleImpl.framework
        │   ├── QtQuickControls2Material.framework
        │   ├── QtQuickControls2MaterialStyleImpl.framework
        │   ├── QtQuickControls2Universal.framework
        │   ├── QtQuickControls2UniversalStyleImpl.framework
        │   ├── QtQuickDialogs2.framework
        │   ├── QtQuickDialogs2QuickImpl.framework
        │   ├── QtQuickDialogs2Utils.framework
        │   ├── QtQuickEffects.framework
        │   ├── QtQuickLayouts.framework
        │   ├── QtQuickParticles.framework
        │   ├── QtQuickShapes.framework
        │   ├── QtQuickTemplates2.framework
        │   ├── QtQuickVectorImage.framework
        │   ├── QtQuickVectorImageGenerator.framework
        │   ├── QtSql.framework
        │   ├── QtSvg.framework
        │   ├── QtWebSockets.framework
        │   ├── QtWidgets.framework
        │   ├── libonnxruntime.dylib
        │   ├── libsherpa-onnx-c-api.dylib
        │   └── libsherpa-onnx-cxx-api.dylib
        ├── Info.plist
        ├── MacOS
        │   ├── gladys
        │   ├── gladys-asr
        │   └── llama
        ├── PlugIns
        │   ├── iconengines
        │   ├── imageformats
        │   ├── networkinformation
        │   ├── platforms
        │   ├── quick
        │   ├── sqldrivers
        │   ├── styles
        │   └── tls
        └── Resources
            ├── icon.icns
            ├── qml
            └── qt.conf

59 directories, 8 files
```

## Signing (ad-hoc, no Apple Developer ID)

Apple Silicon kills unsigned or broken-signature bundles. `scripts/package.sh` seals the bundle as the last step:

```
prune dev files (copy of dist/)
  ▼
codesign -s -  Frameworks/*.dylib, PlugIns/*.dylib, MacOS/* (binaries, llama)
  ▼
codesign -s -  *.framework
  ▼
codesign -s -  gladys.app
  ▼
codesign --verify --deep --strict   (fails the package on error)
  ▼
ditto -c -k --keepParent  ──► gladys-macos-arm64.zip
```

- Any edit to the bundle after signing (including `llama` or `Info.plist`) breaks the seal.
- `ditto` keeps framework symlinks. `zip` does not.
- Users install through `scripts/macos/install.sh`, which checks the SHA-256 and removes quarantine.
