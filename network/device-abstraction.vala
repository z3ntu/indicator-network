// vim: tabstop=4 noexpandtab shiftwidth=4 softtabstop=4
/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Ted Gould <ted.gould@canonical.com>
 */

namespace Network.Device
{

	public class Base : MenuModel {
		NM.Client _client;
		NM.Device _device;
		string _namespace;
		GLibLocal.ActionMuxer _muxer;

		public NM.Device device {
			construct {
				_device = value;
				return;
			}
			get {
				return _device;
			}
		}

		public NM.Client client {
			construct {
				_client = value;
				return;
			}
			get {
				return _client;
			}
		}

		public string namespace {
			construct {
				_namespace = value;
				return;
			}
			get {
				return _namespace;
			}
		}

		public GLibLocal.ActionMuxer muxer {
			construct {
				_muxer = value;
				return;
			}
			get {
				return _muxer;
			}
		}


	}

} // namespace

