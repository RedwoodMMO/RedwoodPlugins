const fs = require("fs");
const path = require("path");
const execSync = require("child_process").execSync;

const version = process.argv[2];
const baseDir = __dirname;

const plugins = [
  "RedwoodChat",
  "RedwoodCore",
  "RedwoodGAS",
  "RedwoodSteam",
];

for (const plugin of plugins) {
  const pluginFile = path.join(baseDir, plugin, `${plugin}.uplugin`);
  const pluginContent = JSON.parse(fs.readFileSync(pluginFile, "utf8"));
  pluginContent.VersionName = version;

  fs.writeFileSync(pluginFile, JSON.stringify(pluginContent, null, 2) + "\n");
}

execSync("git add .", { cwd: baseDir, stdio: "inherit" });

try {
  execSync(`git commit -m "Release ${version}"`, {
    cwd: baseDir,
    stdio: "inherit",
  });
} catch (e) {
  console.log("No changes to commit, already have the correct version");
}

execSync(`git tag ${version}`, { cwd: baseDir, stdio: "inherit" });

const args = [
  "7z",
  "a",
  "-tzip",
  `RedwoodPlugins-${version}.zip`,
  ...plugins,
  "ThirdParty",
  ".clang-format",
  ".gitattributes",
  ".gitignore",
  ".gitmodules",
  "LICENSE.md",
  "README.md",
  "-xr!.git",
  "-xr!.github",
  "-xr!Binaries",
  "-xr!Intermediate",
  "-xr!Saved",
  "-xr!*.zip",
  "-xr!release.js",
];

execSync(args.join(" "), { cwd: baseDir, stdio: "inherit" });
