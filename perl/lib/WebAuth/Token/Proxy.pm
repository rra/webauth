# Perl representation of a WebAuth proxy token.
#
# Written by Russ Allbery <rra@stanford.edu>
# Copyright 2012
#     The Board of Trustees of the Leland Stanford Junior University
#
# See LICENSE for licensing terms.

package WebAuth::Token::Proxy;

require 5.006;
use strict;
use warnings;

use base qw(WebAuth::Token);

# This version should be increased on any code change to this module.  Always
# use two digits for the minor version with a leading zero if necessary so
# that it will sort properly.
our $VERSION = '1.00';

# Accessor methods.
sub subject         { my $self = shift; $self->_attr ('subject',         @_) }
sub type            { my $self = shift; $self->_attr ('type',            @_) }
sub webkdc_proxy    { my $self = shift; $self->_attr ('webkdc_proxy',    @_) }
sub initial_factors { my $self = shift; $self->_attr ('initial_factors', @_) }
sub session_factors { my $self = shift; $self->_attr ('session_factors', @_) }
sub loa             { my $self = shift; $self->_attr ('loa',             @_) }
sub creation        { my $self = shift; $self->_attr ('creation',        @_) }
sub expiration      { my $self = shift; $self->_attr ('expiration',      @_) }

1;