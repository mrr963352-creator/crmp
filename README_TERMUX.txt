Termux theke GitHub e push (PC lagbe na):
  pkg update && pkg install git unzip -y
  termux-setup-storage
  cd ~ && unzip /sdcard/Download/csrp_jni.zip -d csrp_jni && cd csrp_jni
  git init && git add -A && git commit -m build
  git branch -M main
  git remote add origin https://github.com/<USER>/<REPO>.git
  git push -u origin main      # password er jaygay GitHub Personal Access Token (repo + workflow permission)
Tarpor GitHub > Actions > "Build libsamp.so (CSRP)" > run shesh hole Artifacts > libsamp download.
