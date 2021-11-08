[← README](../README.md) 

# Configuration

The Configuration file contains all configuratble and customizable settings for the CSE.

It follows the Windows INI file format with sections, keywords and values. A configuration file may include comments, prefixed with the characters "#"" or ";"" .

## General Usage

A default configuration file is provided with the file [acme.ini.default](../acme.ini.default). Don't make changes to this file, but rather copy it to a new file named *acme.ini*, which is the default configuration file name. You can use another filename, but must then specify it with the *--config* command line argument when running the the (see [Running the CSE](Running.md#running-the-cse)).

### Configuration References

The CSE's configuration module uses the [Python Extended Interpolation Parser](https://docs.python.org/3/library/configparser.html#interpolation-of-values) that implements a more advanced syntax in order to support settings references across the configuration file. That means that you can reference settings in the same or in other configuration sections. See the the [Python documentation](https://docs.python.org/3/library/configparser.html#interpolation-of-values) for a detailed description.

### Using Settings During Imports

Configuration values can be referenced by their respective macro name and used when [importing resources](Importing.md#accessing-configuration-settings).

The following macros are supported in addition to those defined in the sections below:

| Macro name | Description                              |
|:-----------|:-----------------------------------------|
| configfile | Path and name of the configuration file. |

## Configuration Sections

[\[cse\] - General CSE Settings](#general)  
[\[server.security\] - ACP & TLS Settings](#security)  
[\[server.http\] - HTTP Server Settings](#server_http)  
[\[database\] - Database Settings](#database)  
[\[logging\] - Logging Settings](#logging)  
[\[cse.registration\] - Settings for Self-Registrations](#cse_registration)  
[\[cse.registrar\] - Settings for Remote CSE Access](#registrar)  
[\[cse.announcements\] - Settings for Resource Announcements](#announcements)  
[\[cse.statistics\] - Statistic Settings](#statistics)  
[\[cse.resource.acp\] - Resource defaults: ACP](#resource_acp)  
[\[cse.resource.cnt\] - Resource Defaults: CNT](#resource_cnt)  
[\[cse.resource.req\] - Resource Defaults: REQ](#resource_req)  
[\[cse.resource.sub\] - Resource Defaults: SUB](#resource_sub)  
[\[cse.webui\] - Web UI Settings](#webui)  


### Additional Settings
[\[server.http.mappings\] - ID Mappings](#id_mappings)  
[\[app.statistics\] - Configurations for the Statistics AE](#ae_statistics)  
[\[app.csenode\] - Configurations for the CSE Node App](#cse_node)


<a name="general"></a>
### [cse] - General CSE Settings

| Keyword                  | Description                                                                                                                                                                                    | Macro Name                   |
|:-------------------------|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:-----------------------------|
| type                     | The CSE type. Allowed values: IN, MN, ASN.<br/>Default: IN                                                                                                                                     | cse.type                     |
| serviceProviderID        | The CSE's service provider ID.<br/>Default: acme                                                                                                                                               | cse.spid                     |
| cseID                    | The CSE ID. Can be overwritten in imported CSE definition. A CSE-ID must start with a /.<br/>Default: id-in                                                                                    | cse.csi                      |
| resourceID               | The CSE's resource ID. This should be the *cseid* without the leading "/". Can be overwritten in imported CSE definition.<br/>Default: id-in                                                   | cse.ri                       |
| resourceName             | The CSE's resource name or CSE-Name. Can be overwritten in imported CSE definition.<br>Default: cse-in                                                                                         | cse.rn                       |
| resourcesPath            | Directory of default resources to import.<br/>See also command line argument [–import-directory](Running.md).<br/>Default: ./init                                                              | cse.resourcesPath            |
| expirationDelta          | Expiration time before resources are removed in seconds.<br/> Default: 60*60*24*365 = 31536000 = 1 year                                                                                        | cse.expirationDelta          |
| maxExpirationDelta       | Maximum expirationTime allowed for resources in seconds.<br/>Default: 5 years = 157680000                                                                                                      | cse.maxExpirationDelta       |
| originator               | Admin originator for the CSE.<br/>Default: CAdmin                                                                                                                                              | cse.originator               |
| enableApplications       | Enable internal applications. See also individual application configuratins in the [app. ...] sections.<br/>See also command line arguments [–apps and –noapps](Running.md).<br/>Default: true | cse.enableApplications       |
| applicationsStartupDelay | Delay after the CSE startup to run internal applications in seconds.<br/>Default: 5                                                                                                            | cse.applicationsStartupDelay |
| enableNotifications      | Enable notifications.<br/>Default: true                                                                                                                                                        | cse.enableNotifications      |
| enableRemoteCSE          | Enable remote CSE registration and checking.<br/>See also command line arguments [–remote-cse and –no-remote-cse](Running.md).<br/>Default: true                                               | cse.enableRemoteCSE          |
| enableTransitRequests    | Enable forwarding of requests to a remote CSE.<br/>Default: true                                                                                                                               | cse.enableTransitRequests    |
| enableValidation         | Enable the validation of attributes and arguments.<br />Default: true                                                                                                                          | cse.enableValidation         |
| sortDiscoveredResources  | Enable alphabetical sorting of discovery results.<br/>Default: true                                                                                                                            | cse.sortDiscoveredResources  |
| checkExpirationsInterval | Interval to check for expired resources. 0 means "no checking".<br/>Default: 60 seconds                                                                                                        | cse.checkExpirationsInterval |
| flexBlockingPreference   | Indicate the preference for flexBlocking response types. Allowed values: "blocking", "nonblocking".<br />Default: blocking                                                                     | cse.flexBlockingPreference   |
| supportedReleaseVersions | A comma-separated list of supported release versions. This list can contain a single or multiple values.<br />Default: 2a,3,4                                                                  | cse.supportedReleaseVersions |
| releaseVersion           | The release version indicator for requests. Allowed values: 2a, 3, 4.<br />Default: 3                                                                                                    | cse.releaseVersion           |
| defaultSerialization     | Indicate the serialization format if none was given in a request and cannot be determined otherwise.<br/>Allowed values: json, cbor.<br/>Default: json                                         | cse.defaultSerialization     |


<a name="security"></a>
### [cse.security] - General CSE Security Settings

| Keyword           | Description                                                                                                                                                                                                                                      | Macro Name                     |
|:------------------|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:-------------------------------|
| enableACPChecks   | Enable access control checks.<br/> Default: true                                                                                                                                                                                                 | cse.secuerity.enableACPChecks  |
| fullAccessAdmin   | Always grant the admin originator full access (bypass access checks).<br /> Default: True                                                                                                                                                        | cse.security.fullAccessAdmin   |
| useTLS            | Enable TLS for communications.<br />This can be overridden by the command line arguments [--http and --https](Running.md).<br />See oneM2M TS-0003 Clause 8.2.1 "Overview on Security Association Establishment Frameworks".<br />Default: False | cse.security.useTLS            |
| tlsVersion        | TLS version to be used in connections. <br />Allowed versions: TLS1.1, TLS1.2, auto . Use "auto" to allow client-server certificate version negotiation.<br />Default: auto                                                                      | cse.security.tlsVersion        |
| verifyCertificate | Verify certificates in requests. Set to *False* when using self-signed certificates.<br />Default: False                                                                                                                                         | cse.security.verifyCertificate |
| caCertificateFile | Path and filename of the certificate file.<br />Default: None                                                                                                                                                                                    | cse.security.caCertificateFile |
| caPrivateKeyFile  | Path and filename of the private key file.<br />Default: None                                                                                                                                                                                    | cse.security.caPrivateKeyFile  |

<a name="server_http"></a>
###	[server.http] - HTTP Server Settings

| Keyword                   | Description                                                                                                                                                                                                                                                | Macro Name                     |
|:--------------------------|:-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:-------------------------------|
| port                      | Port to listen to.<br/>Default: 8080                                                                                                                                                                                                                       | http.port                      |
| listenIF                  | Interface to listen to. Use 0.0.0.0 for "all" interfaces.<br/>Default:127.0.0.1                                                                                                                                                                            | http.listenIF                  |
| address                   | Own address. Should be a local/public reachable address.<br/> Default: http://127.0.0.1:8080                                                                                                                                                               | http.address                   |
| root                      | CSE Server root. Never provide a trailing /.<br/>Default: empty string                                                                                                                                                                                     | http.root                      |
| multiThread               | Run the http server in single- or multi-threaded mode.<br/> Default: true                                                                                                                                                                                  | http.multiThread               |
| enableRemoteConfiguration | Enable an endpoint for get and set certain configuration values via a REST interface.<br />**ATTENTION: Enabling this feature exposes configuration values, IDs and passwords, and is a security risk.**<br/> Default: false                               | http.enableRemoteConfiguration |
| enableStructureEndpoint   | Enable an endpoint for getting a structured overview about a CSE's resource tree and deployment infrastructure (remote CSE's).<br />**ATTENTION: ATTENTION: Enabling this feature exposes various potentially sensitive information.**<br/> Default: false | http.enableStructureEndpoint   |


<a name="database"></a>
###	[database] - Database Settings

| Keyword        | Description                                                                                                                                                          | Macro Name        |
|:---------------|:---------------------------------------------------------------------------------------------------------------------------------------------------------------------|:------------------|
| path           | Directory for the database files.<br/>Default: ./data                                                                                                                | db.path           |
| inMemory       | Operate the database in in-memory mode. Attention: No data is stored persistently.<br/>See also command line argument [--db-storage](Running.md).<br/>Default: false | db.inMemory       |
| cacheSize      | Cache size in bytes, or 0 to disable caching.<br/>Default: 0                                                                                                         | db.cacheSize      |
| resetOnStartup | Reset the databases at startup.<br/>See also command line argument [--db-reset](Running.md).<br/>Default: false                                                      | db.resetOnStartup |


<a name="logging"></a>
###	[logging] - Logging Settings

| Keyword             | Description                                                                                                                             | Macro Name                  |
|:--------------------|:----------------------------------------------------------------------------------------------------------------------------------------|:----------------------------|
| enable              | Enable logging.<br/>Default: true                                                                                                       | logging.enable              |
| enableFileLogging   | Enable logging to file.<br/>Default: false                                                                                              | logging.enableFileLogging   |
| enableScreenLogging | Enable logging to the screen.<br/>Default: true                                                                                         | logging.enableScreenLogging |
| path                | Pathname for log files.<br />Default: ./logs                                                                                            | logging.path                |
| level               | Loglevel. Allowed values: debug, info, warning, error.<br/>See also command line argument [–log-level](Running.md).<br/> Default: debug | logging.level               |
| count               | Number of files for log rotation.<br/>Default: 10                                                                                       | logging.count               |
| size                | Size per log file.<br/>Default: 100.000 bytes                                                                                           | logging.size                |
| stackTraceOnError   | Print a stack trace when logging an 'error' level message.<br />Default: True                                                           | logging.stackTraceOnError   |


<a name="cse_registration"></a>
###	[cse.registration] - Settings for Self-Registrations

| Keyword               | Description                                                                                                                                                                     | Macro Name                             |
|:----------------------|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:---------------------------------------|
| allowedAEOriginators  | List of AE originators that can register. This is a comma-separated list of originators. Regular expressions are supported.<br />Default: C.\*, S.\*                            | cse.registration.allowedAEOriginators  |
| allowedCSROriginators | List of CSR originators that can register. This is a comma-separated list of originators. Regular expressions are supported.<br />Note: No leading "/"<br />Default: empty list | cse.registration.allowedCSROriginators |
| checkLiveliness       | Check the liveliness if the registrations to the registrar CSE and also from the registree CSEs.<br /> Default: True                                                            | cse.registration.checkLiveliness       |


<a name="registrar"></a>
### [cse.registrar] - Settings for Registrar Registrar CSE Access 

| Keyword              | Description                                                                                                                                          | Macro Name                         |
|:---------------------|:-----------------------------------------------------------------------------------------------------------------------------------------------------|:-----------------------------------|
| address              | URL of the Registrar CSE.<br/>Default: no default                                                                                                    | cse.registrar.address              |
| root                 | Registrar CSE root path. Never provide a trailing /.<br/>Default: empty string                                                                       | cse.registrar.root                 |
| cseID                | CSE-ID of the Registrar CSE. A CSE-ID must start with a /.<br/>Default: no default                                                                   | cse.registrar.csi                  |
| resourceName         | The Registrar CSE's resource name. <br>Default: no default                                                                                           | cse.registrar.rn                   |
| serialization        | Specify the serialization type that must be used for the registration to the registrar CSE.<br />Allowed values: json, cbor<br />Default: json       | cse.registrar.serialization        |
| checkInterval        | Wait n seconds between tries to to connect to the reRegistrarmote CSE and to check validity of Registrar CSE connections in seconds.<br/>Default: 30 | cse.registrar.checkInterval        |
| excludeCSRAttributes | List of resources that are excluded when creating a registrar CSR.<br />Default: empty list                                                          | cse.registrar.excludeCSRAttributes |


<a name="announcements"></a>
### [cse.announcements] - Settings for Resource Announcements 

| Keyword             | Description                                                                                      | Macro Name                            |
|:--------------------|:-------------------------------------------------------------------------------------------------|:--------------------------------------|
| enable              | Enable announcement to remote CSE and allow announced resource registrations.<br />Default: True | cse.announcements.enable              |
| checkInterval       | Wait n seconds between tries to to announce resources to registered remote CSE.<br />Default: 10 | cse.announcements.checkInterval       |



<a name="statistics"></a>
###	[cse.statistics] - Statistic Settings

| Keyword        | Description                                                               | Macro Name                    |
|:---------------|:--------------------------------------------------------------------------|:------------------------------|
| enable         | Enable or disable collecting CSE statistics.<br />Default: True           | cse.statistics.enable         |
| writeInterval  | Interval for saving statistics data to disk in seconds.<br />Default: 60  | cse.statistics.writeInterval |


<a name="resource_acp"></a>
###	[cse.resource.acp] - Resource Defaults: ACP

| Keyword           | Description                                                                   | Macro Name                |
|:------------------|:------------------------------------------------------------------------------|:--------------------------|
| permission        | Default permission when creating an ACP resource.<br />Default: 63            | cse.acp.pv.acop           |
| selfPermission    | Default selfPermission when creating an ACP resource.<br/>Default: 51         | cse.acp.pvs.acop          |


<a name="resource_cnt"></a>
### [cse.resource.cnt] - Resource Defaults: Container

| Keyword      | Description                                            | Macro Name           |
|:-------------|:-------------------------------------------------------|:---------------------|
| enableLimits | Enable/disable the default limits.<br/> Default: False | cse.cnt.enableLimits |
| mni          | Default for maxNrOfInstances.<br/> Default: 10         | cse.cnt.mni          |
| mbs          | Default for maxByteSize.<br/>Default: 10.000 bytes     | cse.cnt.mbs          |


<a name="resource_req"></a>
### [cse.resource.req] - Resource Defaults: Request

| Keyword               | Description                                                                               | Macro Name    |
|:----------------------|:------------------------------------------------------------------------------------------|:--------------|
| minimumExpirationTime | A \<request> resource's minimum expiration time in seconds. Must be >0.<br />Default: 60  | cse.req.minet |
| maximumExpirationTime | A \<request> resource's maximum expiration time in seconds. Must be >0.<br />Default: 180 | cse.req.maxet |


<a name="resource_sub"></a>
### [cse.resource.sub] - Resource Defaults: Subscription

| Keyword             | Description                                                                           | Macro Name  |
|:--------------------|:--------------------------------------------------------------------------------------|:------------|
| batchNotifyDuration | Default for the batchNotify/duration in seconds. Must be >0.<br />Default: 60 seconds | cse.sub.dur |


<a name="webui"></a>
###	[cse.webui] - Web UI Settings

| Keyword | Description                                  | Macro Name       |
|:--------|:---------------------------------------------|:-----------------|
| enable  | Enable the web UI.<br/>Default: true         | cse.webui.enable |
| root    | Root path of the web UI.<br/>Default: /webui | cse.webui.root   |


<a name="id_mappings"></a>
###	[server.http.mappings] - ID Mappings

This section defines mappings for URI paths to IDs in the CSE. Mappings
can be used to provide a more convenient way to access the CSE's resources.
Each setting in the configuration file  specifies a mapping, where the key
specifies a new path and the value specified the mapping to a request
(including optional arguments).

The http server redirects a request to a path element that matches one of 
specified keys to the respective request mapping (using the http status code 307).

Please note, that the "root" path in [server.http](#server_http) prefixes both the new
path and the respecting mapping.

The following snippet only presents some example for ID mappings.

```ini
[server.http.mappings]
/access/v1/devices=/cse-mn?ty=14&fu=1&fo=2&rcn=8
/access/v1/apps=/id-mn?ty=2&fu=1&fo=2&rcn=8
/access/v1/devices/battery=/id-mn?ty=14&mgd=1006&fu=1&fo=2&rcn=8
```

<a name="ae_statistics"></a>
### [app.statistics] - Configurations for the Statistics AE

| Keyword    | Description                                                                                           | Macro Name                |
|:-----------|:------------------------------------------------------------------------------------------------------|:--------------------------|
| enable     | Enable the statistics AE.<br/>Default: true                                                           | app.statistics.enable     |
| aeRN       | Resource name of the statistics AE.<br/>Default: statistics                                           | app.statistics.aeRN       |
| aeAPI      | App-ID of the statistics AE. Must start with "N" or "R".<br/>Default: Ntatistics                      | app.statistics.aeAPI      |
| fcntRN     | Resource name of the statistics flexContainer.<br/> Default: statistics                               | app.statistics.fcntRN     |
| fcntCND    | Content Definition of the AE's flexContainer. This is a proprietary CND.<br/>Default: acme.statistics | app.statistics.fcntCND    |
| fcntType   | Element type of the AE's flexContainer. This is a proprietary type.<br/>Default: acme:csest           | app.statistics.fcntType   |
| originator | Originator for requests to the CSE.<br/>Default: C (client self-registration)                         | app.statistics.originator |
| interval   | Wait n seconds between updates of the AE in seconds.<br/>Default: 10                                  | app.statistics.interval   |


<a name="cse_node"></a>
###	[app.csenode] - Configurations for the CSE Node App

| Keyword             | Description                                                                             | Macro Name                      |
|:--------------------|:----------------------------------------------------------------------------------------|:--------------------------------|
| enable              | Enable the CSE Node.<br/>Default: true                                                  | app.csenode.enable              |
| nodeRN              | Resource name of the CSE Node.<br/>Default: cse-node                                    | app.csenode.nodeRN              |
| nodeID              | Node-ID of the CSE Node.<br/>Default: cse-node                                          | app.csenode.nodeID              |
| originator          | Originator for requests to the CSE.<br/>Default: CAdmin                                 | app.csenode.originator          |
| batteryLowLevel     | Battery level indicates as "low" in percent.<br/>Default: 20                            | app.csenode.batteryLowLevel     |
| batteryChargedLevel | Battery level indicates as "fully charged" in percent.<br/>Default: 100                 | app.csenode.batteryChargedLevel |
| interval            | Wait n seconds between updates of the node and sub-mgmtObjs in seconds.<br/>Default: 60 | app.csenode.interval            |

[← README](../README.md) 
