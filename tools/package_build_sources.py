#!/usr/bin/env python3
"""Bundle this build's sources, licenses and relocatable objects with the BIN."""
from pathlib import Path
import tarfile,sys,os
root=Path(sys.argv[1]).resolve();target=Path(sys.argv[2]).resolve()
framework=Path(os.environ.get('PLATFORMIO_CORE_DIR',str(Path.home()/'.platformio')))/'packages/framework-arduinoespressif8266'
if not (framework/'LICENSE').is_file():raise SystemExit('Framework sources not found')
with tarfile.open(target,'w:gz') as tar:
 for name in ['src','include','tests','tools','LICENSE','LICENSES','THIRD_PARTY_NOTICES.md','RELINKING.md','platformio.ini']:
  path=root/name
  if path.exists():tar.add(path,arcname='project/'+name)
 tar.add(framework,arcname='framework-arduinoespressif8266')
 tar.add(root/'.pio/libdeps/smalltv_ultra',arcname='project/.pio/libdeps/smalltv_ultra')
 tar.add(root/'.pio/build/smalltv_ultra',arcname='project/.pio/build/smalltv_ultra')
print('Build sources, framework, libraries and link objects packaged')
