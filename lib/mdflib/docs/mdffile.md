---
layout: default
title: MDF File
---

# MDF File

The reader and writer store file information in an MDF file object. The file format is 
specified by the ASAM standard organisation and an overview of the file format can be found
here: [ASAM MDF Wiki](https://www.asam.net/standards/detail/mdf/wiki/).

The object contains all information about the MDF file and the user may read and write its properties.
Note that raw data and attachment data is handled by the writer and reader interface as it involves 
reading and writing from the disc.

The following interface exist.

- **IHeader**. Main header (HD) that mainly holds metadata.
- **IFileHistory**. Information (FH) about file changes.<sup>MDF4</sup>
- **IDataGroup**. Defines a measurement (DG) with one or more channel groups.
- **IChannelGroup**. Defines a group of channels (CG) with the same number of samples.
- **IChannel**. Define (CN) a channel.
- **IEvent**. Defines an event (EV).
- **IAttachment**. Reference (AT) to an external or embedded file.<sup>MDF4</sup>
- **IChannelArray**. Defines an channel array (CA).<sup>MDF4</sup>
- **IChannelConversion**. Defines the conversion (CC) from channel value 
to engineering value.
- **IChannelHierarchy**. Defines a channel node hierarchy (CH).<sup>MDF4</sup>
- **IMetaData**. Information (MD) about the test object and test environment.<sup>MDF4</sup>
- **ISourceInformation**. Information (SI) of the data source.<sup>MDF4</sup>
- **ISampleReduction**. Sample reduction (SR) values.<sup>MDF4</sup>

