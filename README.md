# EditAsn1

## Purpose
[Abstract Syntax Notation One (ASN.1)](https://en.wikipedia.org/wiki/Abstract_Syntax_Notation_One) is a standard and notation that describes rules and structures for representing, encoding, transmitting, and decoding data in telecommunications and computer networking. 

This is a handy C application can read such ASN.1 files. The data in these ASN1. files are encoded in triplets of Tag, lengths & values. This application  reads the Tags, lengths and values byte-by-byte and prints them to the console.

It can be used to view/edit TAP3 files and other "unblocked" ASN.1 formats.
It has been coded to identify tags for all TAP3 versions (upto 3.12) and RAP 1.2.

The programs reads the Tags, lengths and values byte-by-byte and prints them to the console.
The Tags ,Lengths & Values are written as (hence u will see T<...> , L<...> and V<...> ). 
Unprintable values are written in Hex, and are enclosed in Vx<...>

This application does not need OSS utilities & has been used extensively to edit EDRs and create test files.

## Compilation
  Use C compiler, ensure that the .c & .h files are in the same folder
  
```sh
  $ cc EditAsn1.c -o EditAsn1
```

## Command line options
The application supports 4 options, just run the application without any parameter to see the options
```
$ EditAsn1
Usage:
        Smart Decode for TAP3     : EditAsn1v15 -d <ASN.1 File>
        Basic/Primitive Decode    : EditAsn1v15 -D <ASN.1 File>
        Smart Encode for TAP3     : EditAsn1v15 -e <Ascii TLV File> <ASN.1 File>
        Basic/Primitive Encode    : EditAsn1v15 -E <Ascii TLV File> <ASN.1 File>
```

## How to Use it to Edit ASN.1 Files
To edit the ASN.1 files, you need to the redirect out of decode to a file.
This file can then be edited by "vi". This does means that the people editing must know a bit about tags & lengths.
Ensure that you change ONLY the value tags (V<...> or Vx<...>) 

**Note** The application does not recalculate lengths, so do not delete Tags-lengths-values & if edited, keep the length same as the original

The edited file can then be given to the application which would write it as a ASN.1 file using the Encoding Options.

The application can easily be changed to work for other ASN.1 definitions which use tags of Application class.

## Customizing for your ASN.1 format
The application included supports TAP3.12, TAP3.11,TAP3.10,TAP3.9 & RAP 1.2.

If you need EditAsn1 to work with create asn1TagInfo.h with information about your ASN.1 file and compile again

## Sample output (TAP3 Notification File , using -d Option)
```
$ ./EditAsn1 -d CDDEUD1AUTPT44999
T<Notification 62> L<0> 
|---T<Sender 5F8144> L<5> V<AUTPT>
|---T<Recipient 5F8136> L<5> V<EUR01>
|---T<FileSequenceNumber 5F6D> L<5> V<00002>
|---T<FileCreationTimeStamp 7F6C> L<0> 
|   +---T<LocalTimeStamp 50> L<14> V<19981030020000>
|   +---T<UtcTimeOffset 5F8167> L<5> V<+0100>
|   +---N<Null 00>
|---T<FileAvailableTimeStamp 7F6B> L<0> 
|   +---T<LocalTimeStamp 50> L<14> V<19981030023000>
|   +---T<UtcTimeOffset 5F8167> L<5> V<+0100>
|   +---N<Null 00>
|---T<TransferCutOffTimeStamp 7F8163> L<0> 
|   +---T<LocalTimeStamp 50> L<14> V<19981029235959>
|   +---T<UtcTimeOffset 5F8167> L<5> V<+0100>
|   +---N<Null 00>
|---T<SpecificationVersionNumber 5F8149> L<1> Vx<03>
|---T<ReleaseVersionNumber 5F813D> L<1> Vx<09>
|---T<FileTypeIndicator 5F6E> L<1> V<T>
|---T<OperatorSpecInfoList 7F8122> L<0> 
|   +---T<OperatorSpecInformation 5F8123> L<12> V<Notification>
|   +---N<Null 00>
|---N<Null 00>
```

## Sample Output ( For CMG file, using -D option)

``` 
$ ./EditAsn1 -D CMG_0001.DAT
T< 30> L<0> 
|---T< A0> L<14> 
|   +---T< 80> L<1> Vx<00>
|   +---T< 81> L<1> Vx<05>
|   +---T< 82> L<1> V<9>
|   +---T< 83> L<3> V<211>
|---T< 81> L<3> Vx<8512F1>
|---T< A2> L<21> 
|   +---T< 80> L<1> Vx<00>
|   +---T< 81> L<1> Vx<01>
|   +---T< 82> L<1> Vx<00>
|   +---T< 83> L<10> V<0724508564>
|---T< 83> L<6> Vx<817042055846>
|---T< 84> L<3> Vx<03010C>
|---T< 85> L<3> Vx<06230D>
|---T< 86> L<1> Vx<04>
|---T< 89> L<1> V<F>
|---T< 8A> L<1> Vx<00>
|---T< AB> L<6> 
|   +---T< 80> L<1> Vx<01>
|   +---T< 81> L<1> Vx<13>
|---T< 8C> L<1> Vx<00>
|---T< 8E> L<1> Vx<00>
|---T< 91> L<1> Vx<00>
|---T< 92> L<1> Vx<02>
|---T< 99> L<3> Vx<03010C>
|---T< 9A> L<3> Vx<06230D>
|---T< BE> L<23> 
|   +---T< 80> L<1> Vx<00>
|   +---T< 81> L<1> Vx<05>
|   +---T< 82> L<1> V<9>
|   +---T< 83> L<12> V<010230169005>
|---T< 9F1F> L<7> Vx<85102003610950>
|---N<Null 00>
T< 30> L<0> 
|---T< A0> L<14> 
|   +---T< 80> L<1> Vx<00>
|   +---T< 81> L<1> Vx<05>
|   +---T< 82> L<1> V<9>
|   +---T< 83> L<3> V<805>
|---T< 81> L<3> Vx<8508F5>
|---T< A2> L<21> 
|   +---T< 80> L<1> Vx<00>
|   +---T< 81> L<1> Vx<01>
|   +---T< 82> L<1> Vx<00>
|   +---T< 83> L<10> V<0723064843>
|---T< 83> L<6> Vx<817032608434>
|---T< 84> L<3> Vx<03010C>
|---T< 85> L<3> Vx<062305>
|---T< 86> L<1> Vx<00>
 ....
 ```
 
