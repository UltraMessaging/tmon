#!/bin/sh
# init_proj.sh

. ../lbm.sh

rm -rf Tmon

mkdir Tmon
cd Tmon
dotnet new sln

dotnet new classlib -o Tmon

dotnet sln add Tmon/Tmon.csproj

rm Tmon/Class1.cs
cp ../../../cs/Tmon/*.cs Tmon/

sed -i.bak -e "/\/PropertyGroup/c\\
\    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>\\
\  </PropertyGroup>\\
\  <ItemGroup>\\
\    <Reference Include=\"lbmcs\">\\
\      <HintPath>$LBMCS_DLL</HintPath>\\
\    </Reference>\\
\  </ItemGroup>" Tmon/Tmon.csproj

# dotnet build

dotnet new console -o TmonExample

dotnet sln add TmonExample/TmonExample.csproj

rm TmonExample/*.cs

cp ../../../cs/TmonExample/*.cs TmonExample/

dotnet add TmonExample/TmonExample.csproj reference Tmon/Tmon.csproj

sed -i.bak -e "/\/PropertyGroup/c\\
\  </PropertyGroup>\\
\  <ItemGroup>\\
\    <Reference Include=\"lbmcs\">\\
\      <HintPath>$LBMCS_DLL</HintPath>\\
\    </Reference>\\
\  </ItemGroup>" TmonExample/TmonExample.csproj
