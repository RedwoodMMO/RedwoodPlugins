const fs = require("fs");
const path = require("path");
const execSync = require("child_process").execSync;

const version = process.argv[2];
const baseDir = __dirname;

const plugins = [
  "RedwoodCore",
  "RedwoodSteam",
];

for (const plugin of plugins) {
  const pluginFile = path.join(baseDir, plugin, `${plugin}.uplugin`);
  const pluginContent = JSON.parse(fs.readFileSync(pluginFile, "utf8"));
  pluginContent.VersionName = version;

  fs.writeFileSync(pluginFile, JSON.stringify(pluginContent, null, 2) + "\n");
}

execSync("git add .", { cwd: baseDir, stdio: "inherit" });
execSync(`git commit -m "Release ${version}"`, {
  cwd: baseDir,
  stdio: "inherit",
});
execSync(`git tag ${version}`, { cwd: baseDir, stdio: "inherit" });
execSync("git push", { cwd: baseDir, stdio: "inherit" });
execSync(`git push origin tag ${version}`, { cwd: baseDir, stdio: "inherit" });

const args = [
  "7z",
  "a",
  "-tzip",
  `RedwoodPlugins-${version}.zip`,
  ".",
  "-xr!.git",
  "-xr!.github",
  "-xr!Binaries",
  "-xr!Intermediate",
  "-xr!Saved",
  "-xr!*.zip",
  "-xr!release.js",
];

execSync(args.join(" "), { cwd: baseDir, stdio: "inherit" });
