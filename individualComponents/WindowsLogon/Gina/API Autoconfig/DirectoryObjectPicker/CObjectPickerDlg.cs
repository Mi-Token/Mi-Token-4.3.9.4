using System;
using System.Collections.Generic;
using System.Text;
using CubicOrange.Windows.Forms.ActiveDirectory;
using System.Security.Principal;

namespace MiToken.ActiveDirectory.GUI
{
    public class CObjectPickerDlg : DirectoryObjectPickerDialog
    {
        public CObjectPickerDlg(bool allowGroups, bool allowUsers)
            : this(allowGroups, allowUsers, false, false)
        {
        }

        public CObjectPickerDlg(bool allowGroups, bool allowUsers, bool allowComputers, bool allowSecPrincipals)
        {
            this.AllowedObjectTypes = ObjectTypes.None;
            if (allowGroups) this.AllowedObjectTypes |= ObjectTypes.Groups | ObjectTypes.BuiltInGroups;
            if (allowUsers) this.AllowedObjectTypes |= ObjectTypes.Users;
            if (allowComputers) this.AllowedObjectTypes |= ObjectTypes.Computers;
            if (allowSecPrincipals) this.AllowedObjectTypes |= ObjectTypes.WellKnownPrincipals;

            this.SkipDomainControllerCheck = false;
            this.ShowAdvancedView = true;
            this.MultiSelect = false;
            this.AllowedLocations = Locations.EnterpriseDomain | Locations.GlobalCatalog | Locations.JoinedDomain | Locations.LocalComputer;
            this.DefaultLocations = Locations.JoinedDomain;
            this.DefaultObjectTypes = ObjectTypes.Users | ObjectTypes.Groups | ObjectTypes.Computers | ObjectTypes.WellKnownPrincipals;
            this.Providers = ADsPathsProviders.LDAP;
            this.AttributesToFetch.Add("objectSID");
            this.AttributesToFetch.Add("objectGUID");
            this.AttributesToFetch.Add("cn");
            this.AttributesToFetch.Add("samAccountName");
            this.AttributesToFetch.Add("email");
        }

        public string GetObjectPath()
        {
            return this.SelectedObject.Path;
        }

        public SecurityIdentifier GetObjectSID()
        {
            byte[] bytes = this.SelectedObject.FetchedAttributes[0] as byte[];
            return new SecurityIdentifier(bytes, 0);
        }

        public Guid GetObjectGuid()
        {
            byte[] bytes = this.SelectedObject.FetchedAttributes[1] as byte[];
            return new Guid(bytes);
        }

        public string GetObjectCN()
        {
            return this.SelectedObject.FetchedAttributes[2] as string;
        }

        public string GetObjectAccountName()
        {
            return this.SelectedObject.FetchedAttributes[3] as string;
        }

        public string GetObjectEmail()
        {
            return this.SelectedObject.FetchedAttributes[4] as string;
        }
    }

    public class CComputerPickerDlg : CObjectPickerDlg
    {
        public CComputerPickerDlg() : base(false, false, true, false) { }
    }
}
