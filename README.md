# EditAsn1

<a href="https://en.wikipedia.org/wiki/Abstract_Syntax_Notation_One">Abstract Syntax Notation One (ASN.1)</a> is a standard and notation that describes rules and structures for representing, encoding, transmitting, and decoding data in telecommunications and computer networking. <br>

This is a handy Tool to read ASN.1 files. The data in these files are encoded in triplets of Tag, lengths & values.<br>

The programs reads the Tags, lengths and values byte-by-byte and prints them to the console.<br>

The Tags ,Lengths & Values are written as (hence u will see T<...> , L<...> and V<...> ). <br>
Unprintable values are written in Hex, and are enclosed in Vx<...><br>

## Compilation
  Use C compiler, ensure that the .c & .h files are in the same folder <br>
  $ cc EditAsn1.c -o EditAsn1


## Sample Output

>  $ ./EditAsn1 -d CDDEUD1AUTPT44999 | head -20<br>
>  T&lt;TransferBatch 61&gt; L&lt;0&gt;<br>
>  |---T&lt;BatchControlInfo 64&gt; L&lt;0&gt;<br>
>  |   +---T&lt;Sender 5F8144&gt; L&lt;5&gt; V&lt;DEUD1&gt;<br>
>  |   +---T&lt;Recipient 5F8136&gt; L&lt;5&gt; V&lt;AUTPT&gt;<br>
>  |   +---T&lt;FileSequenceNumber 5F6D&gt; L&lt;5&gt; V&lt;44298&gt;<br>
>  |   |---T&lt;FileCreationTimeStamp 7F6C&gt; L&lt;0&gt;<br>
>  |   |   +---T&lt;LocalTimeStamp 50&gt; L&lt;14&gt; V&lt;20121116170137&gt;<br>
>  |   |   +---T&lt;UtcTimeOffset 5F8167&gt; L&lt;5&gt; V&lt;+0100&gt;<br>
>  |   |   +---N&lt;Null 00&gt;<br>
>  |   |---T&lt;TransferCutOffTimeStamp 7F8163&gt; L&lt;0&gt;<br>
>  |   |   +---T&lt;LocalTimeStamp 50&gt; L&lt;14&gt; V&lt;20121116170137&gt;<br>
>  |   |   +---T&lt;UtcTimeOffset 5F8167&gt; L&lt;5&gt; V&lt;+0100&gt;<br>
>  |   |   +---N&lt;Null 00&gt;<br>
>  |   |---T&lt;FileAvailableTimeStamp 7F6B&gt; L&lt;0&gt;<br>
>  |   |   +---T&lt;LocalTimeStamp 50&gt; L&lt;14&gt; V&lt;20121116170137&gt;<br>
>  |   |   +---T&lt;UtcTimeOffset 5F8167&gt; L&lt;5&gt; V&lt;+0100&gt;<br>
>  |   |   +---N&lt;Null 00&gt;<br>
>  |   +---T&lt;SpecificationVersionNumber 5F8149&gt; L&lt;1&gt; Vx&lt;03&gt;<br>
>  |   +---T&lt;ReleaseVersionNumber 5F813D&gt; L&lt;1&gt; Vx&lt;0C&gt;<br>
>  |   |---T&lt;OperatorSpecInfoList 7F8122&gt; L&lt;0&gt;<br>


<br> <br>
<p>

## Editing file with the application

To edit a file,<br>
1.  Decode the file<br>
    ./EditAsn1 -d CDDEUD1AUTPT44999 > _TextFile.<br>
2.  Edit _TextFile using vi, make sure you change ONLY the values (things inside V<> or Vx<>) & do not change the length of the values either.<br>
 ..1. You can add/delete TAGs if you can take care of tag-lengths recursively. <br>
3. Encode the file ./EditAsn1 -e _TextFile CDDEUD1AUTPT45000 <br>
4. Verify everything is ok using ./EditAsn1 -d CDDEUD1AUTPT45000<br>
