
for /f "delims=" %%a in (.build_count) do set "fec=%%a"

for /f "skip=1 delims=" %%b in (.build_count) do set "bec=%%b"
git add .
git commit -m "build %fec%/%bec%"
git push