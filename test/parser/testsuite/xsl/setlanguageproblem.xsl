<xsl:stylesheet
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:src="http://www.srcML.org/srcML/src"
	xmlns="http://www.srcML.org/srcML/src"
	xmlns:cpp="http://www.srcML.org/srcML/cpp"
	xmlns:str="http://exslt.org/strings"
	xmlns:func="http://exslt.org/functions"
	xmlns:exsl="http://exslt.org/common"
	extension-element-prefixes="str exsl func"
	exclude-result-prefixes="src cpp"
	version="1.0">

<!--
	setlanguage.xsl

	Converts srcML if statement test cases to while statement test cases

	Michael L. Collard
	collard@cs.kent.edu
-->

<xsl:import href="str.replace.function.xsl"/>

<xsl:output method="xml" omit-xml-declaration="no" version="1.0" encoding="UTF-8" standalone="yes"/>

<xsl:variable name="language" select="/src:unit/@language"/>



<xsl:template match="/src:unit/src:unit">
<xsl:copy>
    <xsl:attribute name="language"><xsl:value-of select="$language"/></xsl:attribute>
    <xsl:apply-templates select="@* | node()"/>
</xsl:copy>
</xsl:template>

<!-- default identity copy -->
<xsl:template match="@*|node()">
	<xsl:copy>
	  <xsl:apply-templates select="@*|node()"/>
	</xsl:copy>
</xsl:template>

</xsl:stylesheet>