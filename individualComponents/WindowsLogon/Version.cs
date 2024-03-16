using System.Reflection;
//
// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version 
//      Build Number
//      Revision
//
// Used by:
//   - customers, Marketing, Presales. E.g. "Our latest version is X.Y.Z" or
//     "In order to get this feature you need to upgrade from your version A.B.C.D1 to A.B.C.D2"
//   - .NET run-time for assembly binding. For strongly named assemblies any change causes
//      different assembly hash to be generated which results in such assembly being rejected by
//      other assemblies not rebuilt to reflect the change.
// Do not alter unless
//   - new release
//   - incompatibility is introduced e.g. interface is changed or data is exchanged 
//     between assemblies in a different format etc.
[assembly: AssemblyVersion("4.3.9.4")]
//
//
// Used by:
//   - customers (for bug reporting), Support
//   - Windows Installer e.g msiexec.
// The build number is not ignored by msiexec and is changed automatically
//   to reflect non-unique Mercurial revision like '1234' e.g. 4.3.1234.0
//   This helps the installer to make correct decisions (upgrade, repair or
//   refuse to downgrade) when new installation package is run.
// The revision number is ignored by msiexec and is set to zero. Change it
//   when releasing a path with a scope of a single or few binaries (so that
//   a customer is not asked to run the whole installation package).
[assembly: AssemblyFileVersion("4.3.8075.0")]                                         // set by Version.js
//
//
//
[assembly: AssemblyInformationalVersion("Version 4.3.9.4, build 8075.072534eca6d8m")]  // set by Version.js
