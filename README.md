# Islandora ArchivesSpace

[![Build Status](https://travis-ci.org/lyrasis/islandora_aspace.svg?branch=7.x)](https://travis-ci.org/lyrasis/islandora_aspace)

## Introduction

This module allows metadata on Islandora to be managed in ArchivesSpace by linking Islandora objects to ArchivesSpace objects. This is accomplished by adding a new ArchivesSpace content model to objects that are being linked to ArchivesSpace objects. 

## Requirements

This module requires the following modules/libraries:

- [Islandora](https://github.com/islandora/islandora)
- [Islandora XML Forms](https://github.com/Islandora/islandora_xml_forms)
- [Drupal Services Module](https://www.drupal.org/project/services)
- [ArchivesSpace Islandora Plugin](https://github.com/lyrasis/aspace-islandora)
- [Twig](https://twig.symfony.com)
- [Composer](https://getcomposer.org)

## Installation

 - Make sure [composer](https://getcomposer.org) is installed and available on the system.
   - Composer is used to install the Twig dependancy.
 - [Install](https://www.drupal.org/docs/7/extending-drupal-7/installing-drupal-7-contributed-modules) Drupal dependancies:
   - [Islandora](https://github.com/islandora/islandora)
   - [Islandora XML Forms](https://github.com/Islandora/islandora_xml_forms)
   - [Drupal Services Module](https://www.drupal.org/project/services)
 - [Install module](https://www.drupal.org/docs/7/extending-drupal-7/installing-drupal-7-contributed-modules) into Drupal.  
 - Run `composer install --no-dev` in module directory.
 - Install Archivesspace plugin on your ASpace server.

## Configuration

- This module has a counterpart plugin for ArchivesSpace that also needs to be configured. Details on configuring that module are available in the [plugins readme file](https://github.com/lyrasis/aspace-islandora/blob/master/README.md#requirements).
- All settings for this module are available at the settings page. It can be found in the menu at: `Islandora > Utility Modules > ArchivesSpace Integration`
- On the *Settings* Tab:
  - On the *General Settings* vertical tab:
    - Enter the ArchivesSpace username and password that Isladora will connect to the API using.
    - Enter the URL for the ArchivesSpace REST API.
    - Copy the API key and use this in ArchivesSpace to configure the integration.
  - On the *Metadata Settings* vertical tab:
    - Choose the `DC Crosswalk` used by the integration to convert the generated `MODS` datastream to Dublin Core for the `DC` datastream. 
    - Choose the `TWIG Template` that will be used to convert the data coming from ArchivesSpace to `MODS`.
  - On the *Content Models* vertical tab
    - Choose the content models that the integration will be applied to.
- On the *Templates* Tab:
  - Here you can *copy*, *edit* or *delete* templates defined for your instance. These templates are used to convert the data coming from ArchivesSpace (found in JSON format in the `ARCHIVESSPACE` datastream) to the `MODS` datastream used in Islandora. 
  - These templates are written using the [TWIG templating system](https://twig.symfony.com/doc/2.x/templates.html). 
  - A very basic template used as the default for the module is available [here](templates/Default.twig). 
  - Pull requests to add more detailed default templates are very welcome.

## Notes

 - For security the REST URLs used to communicate between ArchivesSpace and Islandora should be HTTPS. 
   - If the REST URLS are not HTTPS then the API key and ArchivesSpace Username and Password will be sent in plain text over the internet.
 - This module has been tested with Islandora:
   - 7.x-1.8
   - 7.x-1.10
 - Hook documentation available in the [islandora_aspace.api.php](islandora_aspace.api.php) file.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [Lyrasis](https://github.com/lyrasis)

## Development

If you would like to contribute to this module, please check out [CONTRIBUTING.md](CONTRIBUTING.md). In addition, we have helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the [Islandora.ca](http://islandora.ca) site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
