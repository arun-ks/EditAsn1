# EditAsn1

<a href="https://en.wikipedia.org/wiki/Abstract_Syntax_Notation_One">Abstract Syntax Notation One (ASN.1)</a> is a standard and notation that describes rules and structures for representing, encoding, transmitting, and decoding data in telecommunications and computer networking. <br>

This is a handy Tool to read ASN.1 files. The data in these files are encoded in triplets of Tag, lengths & values.<br><br>

The programs reads the Tags, lengths and values byte-by-byte and prints them to the console.<br>

The Tags ,Lengths & Values are written as (hence u will see T<...> , L<...> and V<...> ). <br>
Unprintable values are written in Hex, and are enclosed in Vx<...><br>

<br>
$ ./EditAsn1 <b>-d</b> CDDEUD1AUTPT44999 | head -20 <br>
T<TransferBatch 61> L<0><br>
|---T<BatchControlInfo 64> L<0><br>
|   +---T<Sender 5F8144> L<5> V<DEUD1><br>
|   +---T<Recipient 5F8136> L<5> V<AUTPT><br>
|   +---T<FileSequenceNumber 5F6D> L<5> V<44298><br>
|   |---T<FileCreationTimeStamp 7F6C> L<0><br>
|   |   +---T<LocalTimeStamp 50> L<14> V<20121116170137><br>
|   |   +---T<UtcTimeOffset 5F8167> L<5> V<+0100><br>
|   |   +---N<Null 00><br>
|   |---T<TransferCutOffTimeStamp 7F8163> L<0><br>
|   |   +---T<LocalTimeStamp 50> L<14> V<20121116170137><br>
|   |   +---T<UtcTimeOffset 5F8167> L<5> V<+0100><br>
|   |   +---N<Null 00><br>
|   |---T<FileAvailableTimeStamp 7F6B> L<0><br>
|   |   +---T<LocalTimeStamp 50> L<14> V<20121116170137><br>
|   |   +---T<UtcTimeOffset 5F8167> L<5> V<+0100><br>
|   |   +---N<Null 00><br>
|   +---T<SpecificationVersionNumber 5F8149> L<1> Vx<03><br>
|   +---T<ReleaseVersionNumber 5F813D> L<1> Vx<0C><br>
|   |---T<OperatorSpecInfoList 7F8122> L<0><br>
<br> <br>
<p>
If you want to edit a file,<br>
1.	Decode the file<br>
    ./EditAsn1 -d CDDEUD1AUTPT44999 > _TextFile.<br>
2.	Edit _TextFile using vi, make sure you change ONLY the values (things inside V<> or Vx<>) & do not change the length of the values either.<br>
    2.1 You can add/delete TAGs if you can take care of tag-lengths recursively. <br>
3.	Encode the file ./EditAsn1 -e _TextFile CDDEUD1AUTPT45000 <br>
4.	Verify everything is ok using ./EditAsn1 -d CDDEUD1AUTPT45000<br>

